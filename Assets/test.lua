-- A simple scene with some miscellaneous geometry.


red = gr.material({0.366046, 0.037182, 0.041638}, {0, 0, 0}, 0, "", "")
green = gr.material({0.162928, 0.408903, 0.083375}, {0, 0, 0}, 0, "", "")
white = gr.material({0.740063, 0.742313, 0.733934}, {0, 0, 0}, 0, "", "")
mat1 = gr.material({0.7, 1.0, 0.7}, {0.5, 0.7, 0.5}, 6.666, "", "")
blue = gr.material({0.0, 0.50980392, 0.50980392}, {0.50196078, 0.50196078, 0.50196078}, 32,"", "")

wood = gr.material({0.5, 0.5, 0.5}, {0, 0, 0}, 0, "wood_texture.png", "wood_normal.png")
brick = gr.material({0.5, 0.5, 0.5}, {0, 0, 0}, 0, "brick_texture.png", "brick_normal.png")

mirror = gr.material({0,0,0}, {1, 1, 1}, 50000000, "", "")


scene_root = gr.node('scene')
-- scene_root:rotate('X', 23)
-- scene_root:translate(6, 2, 15)
-- s1 = gr.nh_sphere('s1', {0, 0, -400}, 100)
-- scene_root:add_child(s1)
-- s1:set_material(mat1)

-- s2 = gr.nh_sphere('s2', {200, 50, -100}, 150)
-- scene_root:add_child(s2)
-- s2:set_material(mat1)

-- s3 = gr.nh_sphere('s3', {0, -1200, -500}, 1000)
-- scene_root:add_child(s3)
-- s3:set_material(mat2)

-- s4 = gr.nh_sphere('s4', {-100, 25, -300}, 50)
-- s4 = gr.nh_sphere('s4', {-150, -085, 150}, 30)
-- scene_root:add_child(s4)
-- s4:set_material(mat3)

-- s5 = gr.nh_sphere('s5', {0, 0, 10}, 5)
-- scene_root:add_child(s5)
-- s5:set_material(red)

-- A small stellated dodecahedron.

steldodec = gr.mesh( 'dodec', 'smstdodeca.obj' )
steldodec:set_material(red)
steldodec:translate(1, 2, 10)
steldodec:scale(0.01, 0.01, 0.01)
scene_root:add_child(steldodec)


plane = gr.mesh( 'plane', 'plane.obj' )
plane:rotate("y", 90)
plane:translate(-1,-1,5)
scene_root:add_child(plane)
plane:set_material(blue)
plane:scale(20, 20, 20)


plane2 = gr.mesh( 'plane2', 'plane.obj' )
plane2:translate(-1,-1,20)
plane2:rotate("z", 90)
plane2:rotate("y", 90)
scene_root:add_child(plane2)
plane2:set_material(white)
plane2:scale(20, 20, 20)

plane3 = gr.mesh( 'plane3', 'plane.obj' )
plane3:translate(5,-1,11)
plane3:rotate("z", 90)
plane3:rotate("y", 180)
scene_root:add_child(plane3)
plane3:set_material(red)
plane3:scale(15, 15, 15)

plane4 = gr.mesh( 'plane4', 'plane.obj' )
plane4:translate(-5,-1,11)
plane4:rotate("z", 90)
plane4:rotate("y", 180)
scene_root:add_child(plane4)
plane4:set_material(green)
plane4:scale(15, 15, 15)

-- plane5 = gr.mesh( 'plane5', 'plane.obj' )
-- plane5:translate(-5,7,11)
-- plane5:rotate("y", 90)
-- scene_root:add_child(plane5)
-- plane5:set_material(red)
-- plane5:scale(15, 15, 15)

s = gr.sphere('s')
scene_root:add_child(s)
s:set_material(mirror)
s:translate(0, 2, 8)

-- teapot =  gr.mesh( 'teapot', 'teapot.obj' )
-- scene_root:add_child(teapot)
-- teapot:set_material(red)
-- teapot:translate(0, 2, 8)


white_light = gr.light({1, 8, 0}, {0.9, 0.9, 0.9}, {1, 0, 0})


gr.render(scene_root, 'test.png', 100, 100,
	  {0, 2, 0}, {0, 2, 5}, {0, 1, 0}, 50,
	  {0.3, 0.3, 0.3}, {white_light})
