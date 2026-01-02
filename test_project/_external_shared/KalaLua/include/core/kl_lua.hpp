//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#pragma once

#include <string>
#include <functional>
#include <vector>
#include <variant>
#include <type_traits>
#include <utility>

extern "C"
{
#include "lua.h"
}

#include "KalaHeaders/core_utils.hpp"
#include "KalaHeaders/log_utils.hpp"

namespace KalaLua::Core
{
	using std::string;
	using std::function;
	using std::vector;
	using std::variant;
	using std::is_same_v;
	using std::index_sequence;
	using std::index_sequence_for;
	using std::get;

	using KalaHeaders::KalaLog::Log;
	using KalaHeaders::KalaLog::LogType;

	using LuaVar = variant
	<
		int,
		float,
		double,
		bool,
		string
	>;

	//Returns false if variable not found in LuaVar is used
	template<typename T>
	constexpr bool IsLuaVarCompatible =
		is_same_v<T, int> 
		|| is_same_v<T, float>
		|| is_same_v<T, double>
		|| is_same_v<T, bool>
		|| is_same_v<T, string>;

	class LIB_API Lua
	{
	public:
		//Initialize KalaLua, does not load scripts or functions
		static bool Initialize();

		static inline bool IsInitialized() { return isInitialized; };

		//Get the pointer to lua state stored within KalaLua
		//after it has initialized, recommended only for advanced users
		static inline lua_State* GetLuaState() { return isInitialized ? state : nullptr; }

		//Load and compile a lua script for use via CallFunction
		static bool LoadScript(const string& script);

		//Call a function from one of the loaded lua scripts with N number of args,
		//empty namespace calls function in global namespace,
		//no dot in namespace calls function in parent namespace,
		//dotted namespace allows nesting namespace calls (my.name.space.function)
		static bool CallFunction(
			const string& functionName,
			const string& functionNamespace,
			const vector<LuaVar>& args = {});

		//Register a function into KalaLua for lua to use externally,
		//accepts N number of any args defined in LuaVar,
		//empty namespace moves function to global namespace,
		//no dot in namespace moves function to parent namespace,
		//dotted namespace allows nesting namespaces (my.name.space)
		template<typename... Args>
		static inline bool RegisterFunction(
			const string& functionName,
			const string& functionNamespace,
			const function<void(Args...)>& targetFunction)
		{
			constexpr bool allArgsSupported = (IsLuaVarCompatible<Args> && ...);

			if constexpr (!allArgsSupported)
			{
				Log::Print(
					"Unsupported variable type was passed to function '" + functionName + "'!",
					"KALALUA_REGISTER_FUNCTION",
					LogType::LOG_ERROR,
					2);

				return false;
			}

			auto invoker = [functionName, targetFunction](const vector<LuaVar>& args)
				{
					if (args.size() != sizeof...(Args))
					{
						Log::Print(
							"Argument count mismatch when invoking function '" + functionName + "'!",
							"KALALUA_REGISTER_FUNCTION",
							LogType::LOG_ERROR,
							2);

						return;
					}

					InvokeTyped(
						targetFunction,
						args,
						index_sequence_for<Args...>{});
				};

			return _RegisterFunction(
				functionName,
				functionNamespace,
				invoker,
				sizeof...(Args));
		}

		//Register a function into KalaLua for lua to use externally,
		//allows custom lua functions, recommended only for advanced users,
		//empty namespace moves function to global namespace,
		//no dot in namespace moves function to parent namespace,
		//dotted namespace allows nesting namespaces (my.name.space)
		static bool RegisterFunction(
			const string& functionName,
			const string& functionNamespace,
			const function<int(lua_State*)>& targetFunction);

		//Shut down KalaLua and the Lua runtime
		static void Shutdown();
	private:
		static inline bool isInitialized{};
		static inline lua_State* state{};

		template<typename... Args, size_t... I>
		static inline void InvokeTyped(
			const function<void(Args...)>& targetFunction,
			const vector<LuaVar>& args,
			index_sequence<I...>)
		{
			targetFunction(get<Args>(args[I])...);
		}

		//The internal true register function that is used
		//to register the function after parsing args
		static bool _RegisterFunction(
			const string& functionName,
			const string& functionNamespace,
			function<void(const vector<LuaVar>&)> invoker,
			size_t argCount);
	};
}