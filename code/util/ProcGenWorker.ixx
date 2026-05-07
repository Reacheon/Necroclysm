export module ProcGenWorker;

import std;

// ════════════════════════════════════════════════════════════════════════
// ProcGenWorker — 절차적 생성 전용 단일 백그라운드 스레드 (싱글톤).
//
//   설계 의도:
//     - 게임 본 로직은 단일 스레드 (시야·AI·턴 처리). 절차생성만 별도 워커 1개로 분리.
//     - 한 섹터 plan은 ~147MB 규모라 메인에서 돌리면 다중초 hitch. 백그라운드 격리 필수.
//     - idle 시 cv.wait → CPU 0%, 메모리 ~1MB.
//     - 활성 시 1스레드 점유 → OS가 다른 hardware thread에 본 게임 로직 분배.
//
//   사용:
//     ProcGenWorker::ins().submit([]{ /* 무거운 작업 */ });
//     // 게임 종료 시
//     ProcGenWorker::ins().shutdown();
//
//   FIFO 큐, 우선순위 없음. 용도가 단순해 priority queue 불필요.
//   향후 도시 layout BCP·강 폴리라인 추출 등도 같은 워커 공유 가능.
// ════════════════════════════════════════════════════════════════════════

export class ProcGenWorker
{
public:
    static ProcGenWorker& ins()
    {
        static ProcGenWorker w;
        return w;
    }

    //작업 큐에 추가. shutdown 후 호출은 무시 (잡 누락은 받아들임 — 종료 경로).
    void submit(std::function<void()> task)
    {
        {
            std::lock_guard lk(mtx_);
            if (stop_.load(std::memory_order_acquire)) return;
            queue_.push(std::move(task));
        }
        cv_.notify_one();
    }

    //현재 큐 + 진행 중 작업 수 — 디버그/로딩 인디케이터용.
    std::size_t pending() const
    {
        std::lock_guard lk(mtx_);
        return queue_.size() + (running_ ? 1 : 0);
    }

    //정상 종료 — 큐 drain 안 함, 진행 중 작업만 마치고 join.
    //  idempotent: 두 번 호출되어도 안전.
    void shutdown() noexcept
    {
        if (stop_.exchange(true)) return;
        cv_.notify_all();
        if (worker_.joinable())
        {
            try { worker_.join(); } catch (...) {}
        }
    }

private:
    ProcGenWorker()
        : worker_([this] { run(); })
    {}

    ~ProcGenWorker()
    {
        shutdown();
    }

    ProcGenWorker(const ProcGenWorker&) = delete;
    ProcGenWorker& operator=(const ProcGenWorker&) = delete;

    void run()
    {
        while (true)
        {
            std::function<void()> job;
            {
                std::unique_lock lk(mtx_);
                cv_.wait(lk, [this] {
                    return stop_.load(std::memory_order_acquire) || !queue_.empty();
                });
                if (stop_.load(std::memory_order_acquire) && queue_.empty()) return;
                job = std::move(queue_.front());
                queue_.pop();
                running_ = true;
            }

            try { job(); } catch (...) {}

            {
                std::lock_guard lk(mtx_);
                running_ = false;
            }
        }
    }

    std::thread                       worker_;
    std::queue<std::function<void()>> queue_;
    mutable std::mutex                mtx_;
    std::condition_variable           cv_;
    std::atomic<bool>                 stop_   { false };
    bool                              running_{ false };
};
