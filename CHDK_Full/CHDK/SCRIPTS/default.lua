--[[
@title Default Script
@chdk_version 1.3
]]

bi=get_buildinfo()
if bi.version=="CHDK" then
    chdk_def_lang=1
else
    chdk_def_lang=2
end

langs     = {}
langs[1]  = {["name"]="ENGLISH",  ["font_cp"]=0,  ["hint"]="CHDK language changed to english"}
langs[2]  = {["name"]="GERMAN",   ["font_cp"]=2,  ["hint"]="CHDK-Sprache auf deutsch geändert"}
langs[13] = {["name"]="RUSSIAN",  ["font_cp"]=1,  ["hint"]="CHDK language changed to russian"}

function get_cam_lang()
    local l
    if get_propset()==1 then
        l=get_prop(196)/256
        if l>7 then l=l+1 end
        if l>22 then l=l+1 end
    else
        l=get_prop(61)/256
    end
    return l+1
end

function get_chdk_lang()
    local l=0
    local lf=get_config_value(64)
    if lf=="" then
        l=chdk_def_lang
    else
        for i,v in ipairs(langs) do
            if string.find(lf, v["name"]..".LNG")~=nil then
                l=i
                break
            end
        end
    end
    return l
end

function file_exists(name)
     local f=io.open(name,"r")
     if f~=nil then io.close(f) return true else return false end
end

chdk_lang=get_chdk_lang()
cam_lang=get_cam_lang()

if cam_lang~=chdk_lang then
    if chdk_lang==0 or cam_lang==chdk_def_lang then
        set_config_value(64,"")
        set_config_value(65,langs[chdk_def_lang].font_cp)
        print(langs[chdk_def_lang].hint)
    elseif langs[cam_lang]~=nil then
        if file_exists("A/CHDK/LANG/"..langs[cam_lang].name..".LNG") then
            set_config_value(64,"A/CHDK/LANG/"..langs[cam_lang].name..".LNG")
            set_config_value(65,langs[cam_lang].font_cp)
            print(langs[cam_lang].hint)
        else
            print(langs[cam_lang].name..".LNG is missing")
        end
    else
        print("unknown language id ("..cam_lang..")")
    end
else
    print(";)")
end;
