export module Corouter;

import std;
import errorBox;  
export struct Corouter
{
    struct promise_type
    {
        std::exception_ptr exception;
        bool is_running = false; 

        Corouter get_return_object() {
            return Corouter(std::coroutine_handle<promise_type>::from_promise(*this));
        }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() { exception = std::current_exception(); }
        void return_void() {}
    };

    std::coroutine_handle<promise_type> handler;
    bool started = false;

    Corouter(std::coroutine_handle<promise_type> inputHandler) : handler(inputHandler) {}

    Corouter(Corouter&& other) noexcept : handler(other.handler), started(other.started) { other.handler = nullptr; }
    Corouter& operator=(Corouter&& other) noexcept
    {
        if (this != &other)
        {
            if (handler) handler.destroy();
            handler = other.handler;
            started = other.started;
            other.handler = nullptr;
        }
        return *this;
    }
    Corouter(const Corouter&) = delete;
    Corouter& operator=(const Corouter&) = delete;

    ~Corouter()
    {
        if (handler && !started) errorBox(L"Corouter destroyed without being started. Use Corouter::start() to run coroutine functions.");
        if (handler && !handler.done() && handler.promise().is_running) errorBox(L"Corouter destroyed while still running.");
        if (handler) handler.destroy();
    }

    // 전역 코루틴 관리
    static inline std::unique_ptr<Corouter> current = nullptr;

    static void start(Corouter&& coro)
    {
        current = std::make_unique<Corouter>(std::move(coro));
        current->run();
    }

    bool done() { return handler.done(); }

    bool isRunning() { return handler && handler.promise().is_running; }

    void run()
    {
        if (handler.promise().is_running) 
        {
            errorBox(L"Corouter::run(): 이미 실행 중인 코루틴입니다!");
            return;
        }

        if (handler.done()) 
        {
            errorBox(L"Corouter::run(): 이미 완료된 코루틴을 다시 실행하려고 합니다!");
            return;
        }

        started = true;
        handler.promise().is_running = true;

        try 
        {
            handler();

            // 실행 후 상태 업데이트
            handler.promise().is_running = false;

            if (handler.promise().exception) {
                std::rethrow_exception(handler.promise().exception);
            }
        }
        catch (...) {
            handler.promise().is_running = false;
            throw;  // 예외를 다시 던짐
        }
    }
};