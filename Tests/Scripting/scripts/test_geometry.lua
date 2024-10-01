local function generateSphere(radius, segments, rings)
	local vertices, indices = {}, {}
	for i = 0, segments do
		local theta = i * math.pi * 2 / segments
		for j = 0, rings do
			local phi = j * math.pi / rings
			local x = radius * math.sin(phi) * math.cos(theta)
			local z = radius * math.sin(phi) * math.sin(theta)
			local y = radius * math.cos(phi)
			table.insert(vertices, { x, y, z })

			local first = (i * (rings + 1)) + j + 1
			local second = first + rings + 1

			if i < segments and j < rings then
				table.insert(indices, first)
				table.insert(indices, second)
				table.insert(indices, first + 1)

				table.insert(indices, second)
				table.insert(indices, second + 1)
				table.insert(indices, first + 1)
			end

		end
	end
	return vertices, indices
end

local perlinNoise = apex.math.generate.perlinNoise

-- Function to generate a large terrain
local function generateTerrain(width, depth, gridSize)
    local vertices = {}
    local indices = {}
    local vertexIndex = 1
    local indexIndex = 1

    -- Number of grid cells
    local cols = math.floor(width / gridSize)
    local rows = math.floor(depth / gridSize)

    -- Preallocate tables for performance
    for i = 1, (cols + 1) * (rows + 1) do
        vertices[i] = {0, 0, 0}
    end

    for i = 1, cols * rows * 6 do
        indices[i] = 0
    end

    -- Generate vertices for the grid
    for i = 0, cols - 1 do
        for j = 0, rows - 1 do
            local x = i * gridSize - width / 2
            local z = j * gridSize - depth / 2
            local pnx = x * width * 64.0
            local pnz = z * depth * 64.0
            local y = perlinNoise(x, z) * 0.8  -- Scale the height by multiplying with a factor

            -- Direct assignment of vertex positions
            vertices[vertexIndex] = {x, y, z}
            vertexIndex = vertexIndex + 1
        end
    end

    -- Generate indices for each grid cell
    for i = 0, cols - 2 do
        for j = 0, rows - 2 do
            local topLeft = i * rows + j 
            local topRight = topLeft + 1
            local bottomLeft = topLeft + rows
            local bottomRight = bottomLeft + 1

            -- First triangle
            indices[indexIndex + 0] = topLeft
            indices[indexIndex + 2] = bottomLeft
            indices[indexIndex + 1] = topRight

            -- Second triangle
            indices[indexIndex + 3] = topRight
            indices[indexIndex + 5] = bottomLeft
            indices[indexIndex + 4] = bottomRight

            indexIndex = indexIndex + 6
        end
    end

    return vertices, indices
end

-- Generate a terrain of size 100x100 with each grid cell being 1x1 in size
local width = 50
local depth = 50
local gridSize = 0.0625

-- Call the terrain generation function
-- local vertices, indices = generateSphere(1, 20, 20)
local vertices, indices = generateTerrain(width, depth, gridSize)

-- Submit vertices and indices to the rendering API
apex.submitVertices(vertices)
apex.submitIndices(indices)

