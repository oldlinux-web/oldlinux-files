/*
 * Set the environment variable PATH from the system PATH
 */
address command
"path > pipe:path"
call open pathout,"pipe:path",'read'
path=""
sep=""
component = readln(pathout)
do while component ~= ""
   select
      when component = "Current_directory" then path = path || sep || "."
      when component = "Ram Disk:" then path = path
      otherwise
         parse var component head ':' rest
         if rest = ""
         then path = path || sep || '/' || head
         else path = path || sep || '/' || head || '/' || rest
   end

   sep = ":"
   component = readln(pathout)
end
call close pathout
'setenv PATH '||path
