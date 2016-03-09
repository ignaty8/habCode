@title MultiMovie 2
@param x minutes per file
@default x 15
   f=0
   print "recording started..."
   press "shoot_half" 
   sleep 1500
   b=1
   l=0
:new
   f=f+1 
   click "shoot_full" 
   if f=1 then release "shoot_half"
   print "starting file :";f
   e=get_tick_count+(x*60000)
   p=get_tick_count+1000
   if b=0 then set_backlight 0
:loop
   if is_pressed "set" then goto "reload"
   if is_pressed "right" then
     if b=0 then  
         b=1   
         set_backlight 1
         sleep 1000
     else
         b=0
         set_backlight 0
         sleep 1000
      endif
   endif
   t=get_tick_count
   if t > e then goto "reload" 
   if t < p then goto "loop"
   p=get_tick_count+1000
   m=(e-t)/60000
   s=((e-t)%60000)/1000
   cls
   print "file:";f,"time left:";m;"m ";s;"s"  
   if b=0 then     
      if l=0 then
         set_led 4 0
         set_led 3 0
         set_led 2 0
         set_led 1 0
         l=4
      else
        l=l-1
        if l=0 then
          set_led 4 1
          set_led 3 1
          set_led 2 1
          set_led 1 1
        endif   
      endif
   endif
   goto "loop"
 
:reload
   print "reloading..." 
   click "shoot_full" 
   do 
     sleep 100 
   until get_movie_status=1
   goto "new"
 
:restore
   if b=0 then set_backlight 1
   if get_movie_status=4 then click "shoot_full"
   print "recording halted"
end