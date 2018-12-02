// TestConsoleApplication.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>

/**
 * This macro outputs the prolog code for a naked intercept function. It
 * should be the first code in the function.
 */
#define INTERCEPT_PROLOG()                          \
	__asm                                           \
    {                                               \
        __asm push    ebp                           \
        __asm mov     ebp,            esp           \
        __asm sub     esp,            __LOCAL_SIZE  \
    }

 /**
  * This macro outputs the epilog code for a naked intercept function. It
  * should be the last code in the function. argsSize is the number of
  * bytes for the arguments to the function (not including the the api parameter).
  * The return from the function should be stored in the "result" variable, and
  * the "stdcall" bool variable determines if the function was called using the
  * stdcall or cdecl calling convention.
  */
#define INTERCEPT_EPILOG(argsSize)                  \
    __asm                                           \
    {                                               \
        __asm mov     eax,            result        \
        __asm cmp     stdcall,        0             \
        __asm mov     esp,            ebp           \
        __asm pop     ebp                           \
        __asm jne     stdcall_ret                   \
        __asm ret     4                             \
    __asm stdcall_ret:                              \
        __asm ret     (4 + argsSize)                \
    }

  /**
   * This macro outputs the epilog code for a naked intercept function that doesn't
   * have a return value. It should be the last code in the function. argsSize is the
   * number of  bytes for the arguments to the function (not including the the api
   * parameter). The "stdcall" bool variable determines if the function was called using
   * the stdcall or cdecl calling convention.
   */
#define INTERCEPT_EPILOG_NO_RETURN(argsSize)        \
    __asm                                           \
    {                                               \
        __asm cmp     stdcall,        0             \
        __asm mov     esp,            ebp           \
        __asm pop     ebp                           \
        __asm jne     stdcall_ret                   \
        __asm ret     4                             \
    __asm stdcall_ret:                              \
        __asm ret     (4 + argsSize)                \
    }

#define INTERCEPT_EPILOG(argsSize)                  \
    __asm                                           \
    {                                               \
        __asm mov     eax,            result        \
        __asm cmp     stdcall,        0             \
        __asm mov     esp,            ebp           \
        __asm pop     ebp                           \
        __asm jne     stdcall_ret                   \
        __asm ret     4                             \
    __asm stdcall_ret:                              \
        __asm ret     (4 + argsSize)                \
    }

#pragma runtime_checks( "s", off )
bool GetIsStdCallConvention(void* function, void* arg1, void** result)
{

	int stackLeftOver;
	int final;

	__asm
	{

		// Remember the stack pointer, so we can check if it got cleaned up.
		mov     stackLeftOver, esp

		// Call the function.
		push    arg1
		call    function

		// Store the result.
		mov     final, eax

		// Compute if the arguments were left on the stack by the function.
		mov     eax, stackLeftOver
		sub     eax, esp
		mov     stackLeftOver, eax

		// Fix the stack.
		add     esp, stackLeftOver

	}

	if (result)
	{
		*result = (void*)final;
	}

	// If anything was left on the stack after we called the function,
	// then it's the cdecl convention.
	return stackLeftOver == 0;


}
#pragma runtime_checks( "s", restore )
#pragma runtime_checks( "s", off )
bool GetIsStdCallConvention(void* function, void* arg1, void* arg2, void** result)
{

	int stackLeftOver;
	int final;

	__asm
	{

		// Remember the stack pointer, so we can check if it got cleaned up.
		mov     stackLeftOver, esp

		// Call the function.
		push    arg2
		push    arg1
		call    function

		// Store the result.
		mov     final, eax

		// Compute if the arguments were left on the stack by the function.
		mov     eax, stackLeftOver
		sub     eax, esp
		mov     stackLeftOver, eax

		// Fix the stack.
		add     esp, stackLeftOver

	}

	if (result)
	{
		*result = (void*)final;
	}

	// If anything was left on the stack after we called the function,
	// then it's the cdecl convention.
	return stackLeftOver == 0;

}
#pragma runtime_checks( "s", restore )
int __stdcall add(int &a, int &b)
{
	return a + b;
}
typedef int (__stdcall *func)(int &, int &);

#pragma auto_inline()
__declspec(naked) void naked_test(func fun, int *a, int *b, void** result, bool& stdcall)
{
	INTERCEPT_PROLOG()
	stdcall = GetIsStdCallConvention((void*)fun, (void*)a, (void*)b, (void**)result);
	INTERCEPT_EPILOG(8)
}
#pragma auto_inline(off)
void test(func fun,int *a, int *b, void** result)
{
	int     _result=0;
	bool    stdcall;
	stdcall = GetIsStdCallConvention((void*)fun, (void*)a, (void*)b, (void**)&_result);
	//naked_test(fun, a, b, (void**)&_result, stdcall);
	*result = (void*)_result;
}

int main()
{
	int a = 1;
	int b = 2;
	func addfun = &add;
	int result = 0;
	//int sum = addfun(a, b);
	test(&add, &a, &b, (void**)&result);
	
	std::cout << "Hello World!\n";
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
