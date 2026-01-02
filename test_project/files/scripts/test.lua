print("[LUA] script loaded")

Test.hello()

function luaHello()
	print("[LUA] hello from lua function")
end

function mathtest(a, b)
	local result = a + b
	print("[LUA] mathtest result: ", result)
	return result
end