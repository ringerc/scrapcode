use <3rdparty/dovetail.scad>
use <../pegboard/pegboard.scad>


render_beam1 = false;
render_beam2 = true;

beam_1_length = 90;
beam_2_length = 40;
beam_height = 19;


/*
beam_1_length = 120;
beam_2_length = 60;
beam_height = 30;
*/
beam_thickness = 3;
hole_diameter = 2.3;
hole_pitch = 4.4;
hole_stagger = true;
beam_1_margins = [8,2.3];
beam_2_margins = [3.0,2.3];

dovetail_max_width = beam_thickness * 1.4;
dovetail_min_width = beam_thickness;
dovetail_height = beam_height;
dovetail_depth = beam_thickness / 2;
dovetail_clearance = 0.25;

dovetail_inset = 2;
dovetail_bottom_margin = 2;

// TODO: add blocks sticking out of long plate with dovetail males in them,
// and female grooves in bottom side of short plate to mate with them.
// Just short, there for alignment/rigidity. Maybe don't even need
// dovetails, just mortise and tenon or simple support triangles
// from short piece?
//
// TODO: add interlocking tabs with screw holes in bases so you can screw them
// together and/or screw them to something.
//
// TODO: add a snapfit on the top so they can stick firmly w/o slipping out
//

if (render_beam1)
{
    union()
    {
        difference()
        {
            pegboard([beam_1_length, beam_height, beam_thickness],
                                hole_diameter, hole_pitch, hole_stagger,
                                beam_1_margins);
                                
            for (i = [0,1])
            {
                translate([(1 - 2*i) * (dovetail_inset + dovetail_max_width/2) + i * beam_1_length,
                           dovetail_bottom_margin,
                           beam_thickness+0.01])
                rotate([-90,0,0])
                female_dovetail_negative(dovetail_max_width, dovetail_min_width, dovetail_depth, dovetail_height, dovetail_clearance);
            }
        }
    }
}

if (render_beam2)
{
    translate([0,beam_height + 5, 0])
    union()
    {
        translate([dovetail_depth,0,0])
        pegboard([beam_2_length - dovetail_depth*2, beam_height, beam_thickness],
                            hole_diameter, hole_pitch, hole_stagger,
                            beam_2_margins);
                            
        for (i = [0,1])
        {
            translate([
                (1 - 2*i) * dovetail_depth + i * beam_2_length,
                       dovetail_bottom_margin,
                       beam_thickness/2])
            rotate([90,(1 - 2*i)*90,180])
            male_dovetail(dovetail_max_width, dovetail_min_width, dovetail_depth, dovetail_height - dovetail_bottom_margin, 0);
                
            // stabilizer blocks
            translate([
                (1 - 2*i) * dovetail_depth  + i * (beam_2_length - dovetail_depth*2),
                0, beam_thickness - 0.01])
            cube([beam_thickness, beam_height, beam_thickness]);
        }
    }
}



/*

dovetail_edge_margin = 5;
dovetail_n_slots = 3;

dovetail_max_width = 3.5;
dovetail_min_width = 2;
dovetail_height = beam_thickness + 1;
dovetail_depth = 3;

// Computed
dovetail_pitch = 
    dovetail_n_slots == 1 ? 0 :
    (beam_height - dovetail_edge_margin*2 - dovetail_max_width) / (dovetail_n_slots - 1);

translate([10,10,0])
difference() {
    pegboard([beam_1_length, beam_height, beam_thickness],
                    hole_diameter, hole_pitch, hole_stagger,
                    beam_margins);
    for (i = [0 : 1 : dovetail_n_slots - 1])
    {
        translate([0,
                dovetail_n_slots == 1 ? beam_height/2
                : dovetail_edge_margin + dovetail_max_width/2
                + i * dovetail_pitch,
            -1])
        rotate([0,0,0])
        female_dovetail_negative(dovetail_max_width, dovetail_min_width, dovetail_depth, dovetail_height, dovetail_clearance);
        
        translate([beam_1_length,
                dovetail_n_slots == 1 ? beam_height/2
                : dovetail_edge_margin + dovetail_max_width/2
                + i * dovetail_pitch,
            -1])
        rotate([0,0, 90])
        female_dovetail_negative(dovetail_max_width, dovetail_min_width, dovetail_depth, dovetail_height, dovetail_clearance);
    }
}
*/
