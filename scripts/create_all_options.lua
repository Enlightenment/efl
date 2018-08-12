options = {
  {"-Dopengl=", "full", "none", "es-egl"},
  {"--buildtype ", "plain", "debug", "release"},
  {"-Devas-modules ", "shared", "static"},
}

concated_options = {}

for i,v in pairs(options) do
  tmp_options = {}

  option_name = v[1]

  for i=2, #v do
    table.insert(tmp_options, option_name..v[i])
  end

  table.insert(concated_options, tmp_options)
end

function permutate(values)
  local permutater = {table.unpack(values[1])}

  if #values == 1 then
    return {table.unpack(values[1])}
  else
    local result = {}
    table.remove(values, 1)
    local list_to_complete = permutate(values)

    for k,v in pairs(list_to_complete) do
      for k_perm,v_perm in pairs(permutater) do
        table.insert(result, v_perm.." "..v)
      end
    end
    return result
  end
end

all_options = permutate(concated_options)

print("GOING TO BUILD ALOT OF EFL")

for k,v in pairs(all_options) do
  cmd = "sh ./scripts/check_options.sh "..v.." "..arg[1]
  exitcode = os.execute(cmd)
  if exitcode ~= true then
    print("command "..cmd.." failed. ")
    print(exitcode)
    os.exit(-1)
  end
end
