a = buffer ('alpha')
blur ({ 5, dst = a })
bump ({ map = a, compensate = true, color = 0x3399FF, specular = 10.0 })
