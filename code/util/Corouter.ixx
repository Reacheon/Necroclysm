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

    Corouter(std::coroutine_handle<promise_type> inputHandler) : handler(inputHandler) {}

    ~Corouter()
    {
        if (handler && !handler.done() && handler.promise().is_running)  errorBox(L"Corouter 소멸자: 아직 실행 중인 코루틴이 삭제되려고 합니다!");
        handler.destroy();
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