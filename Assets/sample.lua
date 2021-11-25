gold = gr.material({0.9, 0.8, 0.4}, {0.8, 0.8, 0.4}, 25)
mirror= gr.material({0.7, 0.4, 0.7}, {0.0, 0.0, 0.0}, 6.666) -- 6.666 shiny value means mirror to my program. 
blue = gr.material({0.8, 0.7, 1}, {0.5, 0.4, 0.8}, 25)

scene = gr.node('scene')
scene:rotate('X', 23)
scene:translate(6, -2, -15)

arc = gr.node('arc')
scene:add_child(arc)
arc:translate(0,0,-10)
arc:rotate('Y', 60)

p1 = gr.cube('p1')
arc:add_child(p1)
p1:set_material(gold)
p1:scale(2, 4, 0.8)
p1:translate(-2.4, 2, -0.4)


s = gr.sphere('s')
arc:add_child(s)
s:set_material(gold)
s:translate(-0.5, 5, 5)

plane = gr.mesh( 'plane', 'plane.obj' )
scene:add_child(plane)
plane:set_material(mirror)
plane:scale(30, 30, 30)

poly = gr.mesh( 'poly', 'dodeca.obj' )
scene:add_child(poly)
poly:translate(-2, 3.618034, 0)
poly:set_material(mirror)


l1 = gr.light({200,200,400}, {0.8, 0.2, 0.8}, {1, 0, 0})
l2 = gr.light({0, 5, -20}, {0.9, 0.4, 0.8}, {1, 0, 0})
l3 = gr.light({0, 5, -10}, {0.3, 0.8, 0.3}, {1, 0, 0})

gr.render(scene, 'sample.png', 756, 756, 
	  {0, 0, 0,}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {l1, l2, l3})
