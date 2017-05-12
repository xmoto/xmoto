tell application "Finder"
	if not (exists item ".xmoto" of home) then
		make new folder at home with properties {name:".xmoto"}
	end if
	open item ".xmoto" of home
end tell
