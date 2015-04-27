local tptr = require "tablepointer"

local sub = { 1,2,3,4 }
local a = { true, false, nil,2,3, a= sub, b= "hello" }
local p = tptr.topointer(a)
tptr.createtable(p)

for _,k,v in tptr.pairs(p) do
	print(k,v)
end
