print("[LUA] script loaded")

function luaHello()
	print("[LUA] hello from lua!")
end

kalalua_test.double_add(5.2435762457, 155.2472345734575)
kalalua_test.float_subtract(55666.4423, 4564.88)
kalalua_test.int_multiply(345, 5566)
kalalua_test.double_divide(667777666.66446666, 33333.333356733)
kalalua_test.string_print("this is string a", "this is string b")

function bool_compare(a, b)
	local result = a > b
	print("[LUA] bool_compare - a is bigger than b: ", result)
	return result
end