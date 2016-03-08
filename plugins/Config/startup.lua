-- Startup script
-- Changes will take effect once Notepad++ is restarted

-- print (123)
editor.EndAtLastLine=false
editor.ViewWS=2

--Virtual space can be enabled or disabled for rectangular selections or in other circumstances or in both. There are two bit flags SCVS_RECTANGULARSELECTION=1 and SCVS_USERACCESSIBLE=2 which can be set independently. SCVS_NONE=0, the default, disables all use of virtual space.
editor.VirtualSpaceOptions=SCVS_RECTANGULARSELECTION
editor.BackSpaceUnIndents=true
editor.AutoCMulti=SC_MULTIAUTOC_EACH --1
--editor:MarkerDefine(int markerNumber, int markerSymbol) 

--editor:CallTipShow(position pos, string definition) 
function RGB(color) return ((color & 0xff) << 16) | (color & 0xff00) | (color >> 16) end

table.print = function ( t )  
    local print_r_cache={}
    local function sub_print_r(t,indent)
        if (print_r_cache[tostring(t)]) then
            print(indent.."*"..tostring(t))
        else
            print_r_cache[tostring(t)]=true
            if (type(t)=="table") then
                for pos,val in pairs(t) do
                    if (type(val)=="table") then
                        print(indent.."["..pos.."] => "..tostring(t).." {")
                        sub_print_r(val,indent..string.rep(" ",string.len(pos)+8))
                        print(indent..string.rep(" ",string.len(pos)+6).."}")
                    elseif (type(val)=="string") then
                        print(indent.."["..pos..'] => "'..val..'"')
                    else
                        print(indent.."["..pos.."] => "..tostring(val))
                    end
                end
            else
                print(indent..tostring(t))
            end
        end
    end
	
    if (type(t)=="table") then
        print(tostring(t).." {")
        sub_print_r(t,"  ")
        print("}")
    else
        sub_print_r(t,"  ")
    end
end

function fi(func)
	if (type(func) ~= "function") then print("not a function") return end
	for key,value in pairs(debug.getinfo(func)) do print(key,value) end
end

function print_t (t, meta)
	if (meta) then 
		if (getmetatable(t)) then
			t = getmetatable(t)
		else
			print("No metatable")
			return
		end
	end
	if (type(t) ~= "table") then print("Not a table") return end
	for key,value in pairs(t) do print(key,value) end 
end
	
--[[
------------------------
local seen={}
function dump(t,i)
    seen[t]=true
    local s={}
    local n=0
    for k in pairs(t) do
        n=n+1 s[n]=k
    end
    table.sort(s)
    for k,v in ipairs(s) do
        print(i,v)
        v=t[v]
        if type(v)=="table" and not seen[v] then
            dump(v,i.."\t")
        end
    end
end
dump(_G,"")
------------------------
function trace (event, line)
  local s = debug.getinfo(2).short_src
  print(s .. ":" .. line)
end
debug.sethook(trace, "l")
------------------------
]]