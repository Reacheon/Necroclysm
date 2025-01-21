print("Hello from Lua!")
print("Value from C++: " .. cppValue)

function multiply(a, b)
    return a * b
end

local product = multiply(3, cppValue)
print("3 * cppValue = " .. product)