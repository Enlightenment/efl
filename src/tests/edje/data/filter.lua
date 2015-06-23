-- Evas filter program

a = buffer { 'alpha' }
b = buffer { src = 'mask' }

padding_set(10)

grow { 5, dst = a }
blur { 6, src = a, color = state.color, ox = 1, oy = 1 }
blur { 2, color = color({cc.r, cc.g, cc.b, cc.a }) }
blend { color = mycolor, ox = 1, oy = 1 }

mask { src = input, mask = b, color = 'cyan', fillmode = 'stretch_y_repeat_x' }

