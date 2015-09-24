--
-- Minimal Development Test
-- Mod: test
--

-- The number of craft recipes to register
local cnt = 20000


local x = minetest.get_us_time()
print('trying batched method...')
local tbl = {}
local ctr = cnt
while ctr ~= 0 do
	table.insert(tbl, {
		output = "test:whatever"..ctr,
		recipe = {
			{"default:cobble", "default:cobble"},
			{"default:cobble", "test:whatever" .. (ctr + 1)},
		}
	})
	ctr = ctr - 1
end
minetest.register_craft_batched(tbl)
local y = minetest.get_us_time()
local batched_time = (y-x)

x = minetest.get_us_time()
print('trying traditional method...')
local ctr = cnt
while ctr ~= 0 do
	minetest.register_craft({
		output = "test:whatever"..ctr,
		recipe = {
			{"default:cobble", "default:cobble"},
			{"default:cobble", "test:whatever" .. (ctr + 1)},
		}
	})
	ctr = ctr - 1
end
y = minetest.get_us_time()
local traditional_time = (y-x)


local batched_advantage = traditional_time - batched_time
local function print_with_per_call(str, val)
	print('time ' .. str .. ' method: ' .. val .. ' us (per call: ' .. val/cnt .. ' us)')
end
print_with_per_call('spent registering using batched', batched_time)
print_with_per_call('spent registering using traditional', traditional_time)
print_with_per_call('advantage of batched', batched_advantage)
