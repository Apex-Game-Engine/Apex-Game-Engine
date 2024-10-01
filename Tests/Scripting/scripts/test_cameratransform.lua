return coroutine.wrap(function (deltaTimeMs)

	local position = { 2.5, 12, 10 }
	local time = 0
	local deltaTime = 0.016
	while true do
		position[1] = 8 * math.sin(time * 0.02)
		position[3] = 8 * math.cos(time * 0.02)
		time = time + deltaTime
		coroutine.yield({ 
			translation = position
		})
	end

end)