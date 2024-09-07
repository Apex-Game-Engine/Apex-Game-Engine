apex_update = nil

function doWalk(state_machine)
	print("doWalk")
	apex.anim.StateMachine.Walk(state_machine)
end

function doRun(state_machine)
	print("doRun")
	apex.anim.StateMachine.Run(state_machine)
end

function doIdle(state_machine)
	print("doIdle")
	apex.anim.StateMachine.Idle(state_machine)
end

function make_anim_states_generator(state_machine)
	local co = coroutine.create(function ()
		for i = 1, 10 do
			doWalk(state_machine)
			coroutine.yield()
			doRun(state_machine)
			coroutine.yield()
		end
		while true do
			doIdle(state_machine)
			coroutine.yield()
		end
	end)
	return function()
		--if coroutine.status(co) ~= "dead" then
			coroutine.resume(co)
		--end
	end
end

function apex_start(state_machine)
	apex_update = make_anim_states_generator(state_machine)
	print("apex_start : apex_update = "..tostring(apex_update))
end

print(apex.anim.StateMachine.Walk)
