SDL3 = {
	basepath = PathDir(ModuleFilename()),

	OptFind = function (name, required)
		local check = function(option, settings)
			option.value = false
			option.use_sdlconfig = false
			option.use_winlib = 0
			option.use_osxframework = false
			option.lib_path = nil
			
			if ExecuteSilent("sdl3-config") > 0 and ExecuteSilent("sdl3-config --cflags") == 0 then
				option.value = true
				option.use_sdlconfig = true
			end
			
			if platform == "win32" then
				option.value = true
				option.use_winlib = 32
			elseif platform == "win64" then
				option.value = true
				option.use_winlib = 64
			end
			
			if platform == "macosx" then
				option.value = true
				option.use_osxframework = true
				option.use_sdlconfig = false
			end
		end
		
		local apply = function(option, settings)
			if option.use_sdlconfig == true then
				settings.cc.flags:Add("`sdl3-config --cflags`")
				settings.link.flags:Add("`sdl3-config --libs`")
			end

			if option.use_osxframework == true then
				client_settings.link.frameworks:Add("SDL3")
				client_settings.cc.includes:Add("/Library/Frameworks/SDL3.framework/Headers")
			end

			if option.use_winlib > 0 then
				settings.cc.includes:Add(SDL3.basepath .. "/include")
				if option.use_winlib == 32 then
					settings.link.libpath:Add(SDL3.basepath .. "/lib32")
				else
					settings.link.libpath:Add(SDL3.basepath .. "/lib64")
				end
				settings.link.libs:Add("SDL3")
				settings.link.libs:Add("SDL3main")
			end
		end
		
		local save = function(option, output)
			output:option(option, "value")
			output:option(option, "use_sdlconfig")
			output:option(option, "use_winlib")
			output:option(option, "use_osxframework")
		end
		
		local display = function(option)
			if option.value == true then
				if option.use_sdlconfig == true then return "using sdl3-config" end
				if option.use_winlib == 32 then return "using supplied win32 libraries" end
				if option.use_winlib == 64 then return "using supplied win64 libraries" end
				if option.use_osxframework == true then return "using osx framework" end
				return "using unknown method"
			else
				if option.required then
					return "not found (required)"
				else
					return "not found (optional)"
				end
			end
		end
		
		local o = MakeOption(name, 0, check, save, display)
		o.Apply = apply
		o.include_path = nil
		o.lib_path = nil
		o.required = required
		return o
	end
}
