--[[
A simple 'color' class for Evas filters.

The default alpha value will be 255 unless specified,
which means the default color is opaque black.

r,g,b,a values range from 0 to 255 and are straight
(ie. NOT pre-multiplied).
--]]

local __color, __inrange, __color_parse

--[[Checks that a number is valid and in the range 0-255]]
__inrange = function(a)
   if ((not tonumber(a)) or (tonumber(a) < 0) or (tonumber(a) > 255)) then
      return false
   else
      return true
   end
end

--[[
Parses a string of one of the formats:
 1. "#RRGGBB"
 2. "#RRGGBBAA"
 3. "#RGB"
 4. "#RGBA"
 To the rgba values.
Same as evas_common_format_color_parse, except we don't premultiply here.
--]]
__color_parse = function(str)
   local r,g,b,a
   if not str then return 0,0,0,0 end
   if not string.match(str, "^#[%x]+$") then return 0,0,0,0 end
   len = string.len(str)
   if len == 7 then -- #rrggbb
      r = tonumber(string.sub(str, 2, 3), 16)
      g = tonumber(string.sub(str, 4, 5), 16)
      b = tonumber(string.sub(str, 6, 7), 16)
      a = 0xff
      return r,g,b,a
   end
   if len == 9 then -- #rrggbbaa
      r = tonumber(string.sub(str, 2, 3), 16)
      g = tonumber(string.sub(str, 4, 5), 16)
      b = tonumber(string.sub(str, 6, 7), 16)
      a = tonumber(string.sub(str, 8, 9), 16)
      return r,g,b,a
   end
   if len == 4 then -- #rgb
      r = tonumber(string.sub(str, 2, 2), 16)
      g = tonumber(string.sub(str, 3, 3), 16)
      b = tonumber(string.sub(str, 4, 4), 16)
      r = (r * 0x10) + r
      g = (g * 0x10) + g
      b = (b * 0x10) + b
      a = 0xff
      return r,g,b,a
   end
   if len == 5 then -- #rgba
      r = tonumber(string.sub(str, 2, 2), 16)
      g = tonumber(string.sub(str, 3, 3), 16)
      b = tonumber(string.sub(str, 4, 4), 16)
      a = tonumber(string.sub(str, 5, 5), 16)
      r = (r * 0x10) + r
      g = (g * 0x10) + g
      b = (b * 0x10) + b
      a = (a * 0x10) + a
      return r,g,b,a
   end
   return 0,0,0,255
end

__color = {
   __names = {
      white = 0xFFFFFFFF,
      black = 0xFF000000,
      red = 0xFFFF0000,
      green = 0xFF008000,
      blue = 0xFF0000FF,
      darkblue = 0xFF0000A0,
      yellow = 0xFFFFFF00,
      magenta = 0xFFFF00FF,
      cyan = 0xFF00FFFF,
      orange = 0xFFFFA500,
      purple = 0xFF800080,
      brown = 0xFFA52A2A,
      maroon = 0xFF800000,
      lime = 0xFF00FF00,
      gray = 0xFF808080,
      grey = 0xFF808080,
      silver = 0xFFC0C0C0,
      olive = 0xFF808000,
      invisible = '#0000',
      transparent = '#0000'
   },

   __methods = {
      --[[
      Assign a value to a color object.

      Accepted formats include:
       - 'colorname' (eg. 'red')
       - another color object
       - {r,g,b} or {r,g,b,a}
       - a single integer value from 0x00000000 to 0xFFFFFFFF (0xAARRGGBB)
       - 3 or 4 arguments (c:set(r,g,b) or c:set(r,g,b,a))
       - a string like "#aarrggbb"
      --]]
      set = function (self, A, B, C, D)
         -- nil
         if not A then
            return self:set(0xFF000000)
         end

         -- name or #value or 0xvalue
         if (type(A) == 'string') then
            if string.sub(A, 1, 1) == "#" then
               return self:set(__color_parse(A))
            end
            if string.sub(A, 1, 2) == "0x" then
               return self:set(tonumber(A))
            end
            return self:set(__color.__names[A])
         end

         -- another color
         if (getmetatable(A) == getmetatable(self)) then
            self.r = math.floor(A.r)
            self.g = math.floor(A.g)
            self.b = math.floor(A.b)
            self.a = math.floor(A.a)
            return self
         end

         -- input {r,g,b} or {r,g,b,a}
         if (type(A) == 'table') then
            if ((not __inrange(A[1])) or (not __inrange(A[2])) or (not __inrange(A[3]))) then
               error('Invalid color value: ' .. tostring(A[1]) .. " , " .. tostring(A[2]) .. " , " .. tostring(A[3]))
            end
            self.r = math.floor(A[1])
            self.g = math.floor(A[2])
            self.b = math.floor(A[3])
            if (__inrange(A[4])) then self.a = math.floor(A[4]) else self.a = 255 end
            return self
         end

         -- input single value 0xAARRGGBB
         if ((B == nil) and (type(A) == 'number')) then
            A = math.floor(A)
            if ((A < 0) or (A > 0xFFFFFFFF)) then
               error('Invalid color value: ' .. string.format("0x%x", A))
            end
            self.a = math.floor(A / 0x1000000)
            self.r = math.floor((A / 0x10000) % 0x100)
            self.g = math.floor((A / 0x100) % 0x100)
            self.b = math.floor(A % 0x100)
            if (self.a == 0) then self.a = 255 end
            return self
         end

         -- simplest method (r,g,b[,a])
         if ((not __inrange(A)) or (not __inrange(B)) or (not __inrange(C))) then
            error('Invalid color value: ' .. tostring(A) .. " , " .. tostring(B) .. " , " .. tostring(C))
         end
         if (__inrange(D)) then self.a = math.floor(D) else self.a = 255 end
         self.r = math.floor(A)
         self.g = math.floor(B)
         self.b = math.floor(C)
         return self
      end,

      --[[
      Multiply a color by a value (another color or an alpha value).
      Returns a new value.
      --]]
      mul = function (self, A)
         local C = __color(self)
         if tonumber(A) ~= nil then
            C.a = C.a * tonumber(A) / 255
         else
            A = __color(A)
            C.r = C.r * A.r / 255
            C.g = C.g * A.g / 255
            C.b = C.b * A.b / 255
            C.a = C.a * A.a / 255
         end
         return C
      end,

      --[[
      Add a color to another.
      Returns a new value.
      --]]
      add = function (self, A)
         local C = __color(self)
         A = __color(A)
         C.a = math.min(C.a + A.a, 255)
         C.r = math.min(C.r + A.r, 255)
         C.g = math.min(C.g + A.g, 255)
         C.b = math.min(C.b + A.b, 255)
         return C
      end,

      --[[
      Alpha blending function: A:blend(B) returns A.a*A.rgb + B.a*(255-A.a)*B.rgb
      This blends A on top of B.
      Returns a new value.
      --]]
      blend = function (self, A)
         local C = __color(self)
         A = __color(A)
         C.r = ((C.a * C.r) / 255) + ((255 - C.a) * A.a) * A.r / (255 * 255);
         C.g = ((C.a * C.g) / 255) + ((255 - C.a) * A.a) * A.g / (255 * 255);
         C.b = ((C.a * C.b) / 255) + ((255 - C.a) * A.a) * A.b / (255 * 255);
         C.a = C.a + ((255 - C.a) * A.a) / 255;
         return C
      end
   },

   __index = function (self, key)
      methods = getmetatable(self).__methods
      if (rawget(methods, key)) then return rawget(methods, key) end
      error('Invalid index \'' .. tostring(key) .. '\' for a color')
   end,

   __tostring = function (self)
      return string.format('#%02x%02x%02x%02x', self.r, self.g, self.b, self.a)
   end,

   __call = function (mt, ...)
      local C = {}
      setmetatable(C, mt)
      return C:set(...)
   end,

   __mul = function (self, ...)
      return __color(self):mul(...)
   end,

   __add = function (self, ...)
      return __color(self):add(...)
   end,

   -- Register all global values into global env (_G)
   __register = function ()
      for k, v in pairs(__color.__names) do
         rawset(_G, k, __color(v))
      end
   end,

   -- Test case
   __test = function ()
      local A, B, C

      C = __color()
      assert(tostring(C) == '#000000ff')
      C:set({0xFE, 0xAB, 0x12})
      assert(tostring(C) == '#feab12ff')
      C:set(0xFFFEAB99)
      assert(tostring(C) == '#feab99ff')
      C:set()
      assert(tostring(C) == '#000000ff')
      C:set(0xfe, 0xab, 0x12, 0xff)
      assert(tostring(C) == '#feab12ff')
      C = __color{0xfe, 0xab, 0x12}
      assert(tostring(C) == '#feab12ff')
      B = __color(C)
      assert(tostring(B) == '#feab12ff')
      B = B * 128
      assert(tostring(B) == '#feab1280')
      A = B * C
      assert(tostring(A) == '#fd720180')
      A = B + C
      assert(tostring(A) == '#ffff24ff')
      A = __color(0xFF012345):blend(0xFFFFFFFF)
      assert(tostring(A) == '#012345ff')
      A = __color(0x00012345):blend(0xFFFFFFFF)
      assert(tostring(A) == '#ffffffff')
      A = __color(0x80102030):blend(0xFFFFFFFF)
      assert(tostring(A) == '#878f97ff') -- check this
      A = __color(0x80102030):blend("transparent")
      assert(tostring(A) == '#08101880')
      A = __color("#ff0000ff") * 255
      assert(tostring(A) == '#ff0000ff')
      A = A * 0x80
      assert(tostring(A) == '#ff000080')
      assert(tostring(__color('#123')) == '#112233ff')
      assert(tostring(__color('#1234')) == '#11223344')
      assert(tostring(__color('#123456')) == '#123456ff')
      assert(tostring(__color('#12345678')) == '#12345678')

      __color.__register()
      assert(tostring(white) == '#ffffffff')
      assert(tostring(red) == '#ff0000ff')

      print('All color tests passed')
      return true
   end
}
setmetatable(__color, __color)
if arg and arg[1] == "-t" then __color.__test() end
return __color
