

function __httpHandle(request, response) 

	print("== REQUEST BEGIN ==")
	print("method=" .. request.method)
	print("path=" .. request.path)

	for k,v in pairs(request.queryParams) do
		print("[param] " .. k .. ' = ' .. v)
	end
	for k,v in pairs(request.headers) do
		print("[header] " .. k .. ' = ' .. v)
	end
	
	-- sample output
	response.content = "Test"
	response.code = 404
	response.headers["X-Test"] = "123"
	
	print("== REQUEST END ==")
end


