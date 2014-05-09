a = buffer ('alpha')
blur ({ 4, dst = a })
curve ({ points = '0:0 - 20:0 - 60:255 - 160:255 - 200:0 - 255:0', src = a, dst = a })
blend ({ src = a, color = 'black' })
