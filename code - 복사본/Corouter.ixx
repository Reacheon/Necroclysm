export module Corouter;

import std;

export struct Corouter
{
	struct promise_type
	{
		std::exception_ptr exception;
		Corouter get_return_object() { return Corouter(std::coroutine_handle<promise_type>::from_promise(*this)); }
		std::suspend_always initial_suspend() { return {}; }
		std::suspend_always final_suspend() noexcept { return {}; }
		void unhandled_exception() { exception = std::current_exception(); }
		void return_void() {}
	};
	std::coroutine_handle<promise_type> handler;
	Corouter(std::coroutine_handle<promise_type> inputHandler) : handler(inputHandler) {}
	~Corouter() { handler.destroy(); }
	bool done() { return handler.done(); }
	void run()
	{
		handler();
		if (handler.promise().exception) { std::rethrow_exception(handler.promise().exception); }
	}
};
