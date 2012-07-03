import af

job = af.Job( "test_rmt" )
block = af.Block( 'test_rmt', 'prman')
#block.setWorkingDirectory( '//nimitz/EDIT/_PROJECTS/_Semki/Semki_Erema/3D_scenes/maya' )
block.setCommand( 'prman -progress -t:1 //nimitz/EDIT/_PROJECTS/_Semki/Semki_Erema/3D_scenes/maya/rib/TST/test_remote_perspShape.%04d.rib' )

#block.setWorkingDirectory( '/hosts/nimitz/mnt/ARECA/_EDIT/_PROJECTS/_Semki/Semki_Erema/3D_scenes/maya' )
#block.setCommand( 'prman -progress -t:1 /hosts/nimitz/mnt/ARECA/_EDIT/_PROJECTS/_Semki/Semki_Erema/3D_scenes/maya/rib/TST/test_remote_perspShape.%04d.rib' )

block.setNumeric( 1, 24, 4 ) 
job.blocks.append( block )



print
job.output( True )
print

job.send()