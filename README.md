# KalaLua

**KalaLua** is a C++ 20 wrapper library around Lua for Windows (Linux coming in the future) that allows you to route Lua calls to one middleman library without needing Lua itself in any way inside your own program.

An example project showcasing how to use **KalaLua** has been created at the [kalakit_test_projects repository](https://github.com/greeenlaser/kalakit_test_projects/tree/main/KalaLua).

## Features

### Easy initialization and shutdown

Just call Lua::Initialize to start and Lua::Shutdown to clean up resources and quit.

### Clean from Lua source code

The Lua source code is compiled to the Lua binary but your process does not need to link to it or use any of its source code unless you wish to access the Lua state pointer. Only the KalaLua binary depends on the Lua binary.

### LuaVar-bound arguments 

Passed arguments are safely bound-checked against the LuaVar variant.
Allowed arguments:
- int
- float
- double
- bool
- string

### Three namespace states

You can call and register functions with no namespace, single parent namespace or nested namespace.

Pass "" to the namespace field to not assign a namespace, pass "example" to assign a single parent namespace, pass "example.examplenest" to assign a nested namespace.

### Arguments

Both CallFunction and the three non-lua RegisterFunction functions accept any number of arguments, or none, as long as theyre within the LuaVar variant range.

### Three ways to register functions for use in Lua

Lua::RegisterFunction accepts functionals and free functions and can return void or one of the LuaVar variables.

There is also a fourth advanced caller which expects you to handle lua_State, this is only recommended for advanced users as it requires you to link against the Lua dll and use Lua source code.

### Two ways to call Lua functions

You can call a Lua function with Lua::CallFunction which returns nothing or one of the possible LuaVar variables.

---

## Links

[Donate on PayPal](https://www.paypal.com/donate/?hosted_button_id=QWG8SAYX5TTP6)

[Official Discord server](https://discord.gg/jkvasmTND5)

[Official Youtube channel](https://youtube.com/greenlaser)

---

## Docs

[How to build from source](docs/build_from_source.md)

[External libraries](docs/external_libraries.md)

[Lost Empire Entertainment and KalaKit ecosystem](docs/ecosystem.md)
