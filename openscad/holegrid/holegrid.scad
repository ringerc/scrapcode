// Length (mm) of board section (x axis)
board_length = 120;
// Width (mm) of board section (y axis)
board_width = 21;
// Depth (mm) of board section (z axis)
board_thickness = 3;

// Diameter of holes in board
hole_diameter = 4.0;
// Hole pitch (distance between centers of holes)
hole_pitch = 3.9;
// Should holes be in a packed triangular environment (true) or a regular grid (false)
hole_stagger = true;
// Margins around the edges of the board where no holes should be placed. Can be a 2-array [x,y] or a scalar.
beam_margins = hole_pitch/2;

/*
 * Example usage:
 *
 * track_strip([board_length, board_width, board_thickness],
 *              hole_diameter, hole_pitch, hole_stagger,
 *              beam_margins);
 *              
 */

// Stop Customiser seeing the rest of the params
module dummy() {
}

// Hole generation quality
$fn = 25;

// Set to true if you want to run the rests instead of building the
// specified object.
run_tests = true;

module track_strip(t_dims, t_hole_diam, t_pitch, t_stagger, t_margins, t_hole_depth)
{
    t_length = t_dims[0];
    t_height = t_dims[1];
    t_thickness = t_dims[2];
    t_hole_depth = 
        t_hole_depth == undef ? t_thickness: t_hole_depth; 
    // t_margins can be an xy array or a scalar, or unset
    margin_x = t_margins == undef ? t_hole_diam/2 :
        t_margins[0] == undef ? t_margins : t_margins[0];
    margin_y = t_margins[1] == undef ? margin_x : t_margins[1];
    // X gap between rows
    xpitch = t_pitch;
    /*
     * Y gap between cols. If staggered then there are equilateral
     * triangles between centers of each row; the pitch is the
     * hypotenuse so the y-gap is the long side of the triangle
     * formed between the y-axis and the line between the centers.
     */
    ypitch = xpitch * (t_stagger ? sin(60) : 1);
    nrows = 1 + max(0, floor(
        (t_height - margin_y * 2 - t_hole_diam)/ypitch));
    ncols = 1 + max(0, floor(
        (t_length - margin_x * 2 - t_hole_diam)/t_pitch));
    /*
     * Compute bridge size for reporting use. Bridge min
     * size should not vary based on stagger since we x-offset
     * to compensate for the closer rows.
     */
    bridge_size = (t_pitch - t_hole_diam);
    
    echo(str("strip ", t_length, "x", t_height, " margins ", margin_x, "x", margin_y, " rows ", nrows, " cols ", ncols, , " holes diameter ", t_hole_diam, (t_stagger ? " staggered" : " straight"), " at pitch ", t_pitch, " (bridges ", bridge_size, ")"));
    
    difference()
    {
        cube([t_length, t_height, t_thickness]);
        
        translate([margin_x + t_hole_diam/2,
                   margin_y + t_hole_diam/2, 0])
        
        color("#CCCCCCCC")
        for (r = [0 : 1 : nrows - 1])
        {
            for (i = [0 : 1 : ncols - 1])
            {   
                /* 
                 * For staggered holes, t_ncols is computed to fit
                 * the non-offset row within the margins. The last
                 * hole of the offset row won't fit and must be
                 * omitted.
                 */
                if (!(t_stagger && r % 2 == 1 && i == ncols - 1))
                {
                    // Defend against floating point issues with holes
                    // that fully punch out of the object by pushing
                    // them right through. Extra height will be added
                    // to the hole being punched to compensate.
                    translate([0,0,
                        t_hole_depth >= t_thickness ? -1 : t_thickness - t_hole_depth])
                    // Handle row stagger
                    translate(
                        t_stagger ?
                        /*
                         * Rows are staggered so the y-offset is
                         * sin(45deg) of the pitch and every second
                         * row is inset by half the pitch.
                         */
                        [(r % 2)*xpitch/2 + i*xpitch, r * ypitch, 0]
                        :
                        // Not staggered, regular grid
                        [i*xpitch, r * ypitch, 0]
                    )
                    // We need extra height to ensure we punch out the top
                    // and (if the depth is enough that we translated
                    // the cutting cylinder down, above) below.
                    cylinder(d=t_hole_diam,
                             h=t_hole_depth + 2);
                }
            }
        }
    };
    
    
    /*
     * OpenSCAD doesn't let us assign values to outer scopes. Nor does
     * it let us pass functions as objects. So to allow assertions etc,
     * permit definition of child objects here but discard the product.
     *
     * There must be a better way to do this; what we really want to
     * do is pass a function to evaluate, or export some values to
     * the outer scope.
     */
    $strip_ncols = ncols;
    $strip_nrows = nrows;
    children();
    
}

// Hint: use F12 "thrown together" view for tests
module test()
{

    // Coarse draft for tests; we don't need quality and we're going
    // to have a LOT of holes.
    $fn = 10;
    
    //=== BASIC FORMS ===
    
    // Default margin, stagger
    track_strip([20, 20, 3], 1, 2, true) {
        assert($strip_ncols == 10);
        assert($strip_nrows == 11);
    }
    
    // Scalar margin, stagger
    translate([25, 0, 0])
    track_strip([20, 20, 3], 1, 2, true, 4) {
        assert($strip_ncols == 6);
        assert($strip_nrows == 7);
    }
    
    // Vector margin, stagger
    translate([50, 0, 0])
    track_strip([20, 20, 3], 1, 2, true, [2, 4]) {
        assert($strip_ncols == 8);
        assert($strip_nrows == 7);
    }
    
    // Default margin, no stagger
    translate([0, 25, 0])
    track_strip([20, 20, 3], 1, 2, false) {
        assert($strip_ncols == 10);
        assert($strip_nrows == 10);
    }
    
    // Scalar margin, no stagger
    translate([25, 25, 0])
    track_strip([20, 20, 3], 1, 2, false, 4) {
        assert($strip_ncols == 6);
        assert($strip_nrows == 6);
    }
    
    // Vector margin, no stagger
    translate([50, 25, 0])
    track_strip([20, 20, 3], 1, 2, false, [2, 4])
    {
        assert($strip_ncols == 8);
        assert($strip_nrows == 6);
    }
    
    // === SMALL FORMS ===
    
    // Should produce 1 row, 1 col, as diameter is 1, margin total is 1,
    // height is 2
    translate([0, 50, 0])
    track_strip([2, 2, 3], 1, 2, false, 0.5) {
        assert($strip_ncols == 1);
        assert($strip_nrows == 1);
    };
    
    // With 1 hole, stagger doesn't matter so same result
    translate([10, 50, 0])
    track_strip([2, 2, 3], 1, 2, true, 0.5) {
        assert($strip_ncols == 1);
        assert($strip_nrows == 1);
    };
    
    // This should have 4 holes
    translate([20, 50, 0])
    track_strip([4, 4, 3], 1, 2, false, 0.5) {
        assert($strip_ncols == 2);
        assert($strip_nrows == 2);
    };
    
    // This should produce 3 holes. The bottom row isn't
    // affected by stagger.
    translate([30, 50, 0])
    track_strip([4, 4, 3], 1, 2, true, 0.5) {
        assert($strip_ncols == 2);
        assert($strip_nrows == 2);
    };
    
    // === Shallow holes ====
    
    // Holes in this one only go half way through
    translate([40, 50, 0])
    track_strip([2, 2, 3], 1, 2, false, 0.5, 1.5);
    
    
    // === TIGHT PACKING ===
    
    // No stagger. Holes should touch, bridge ~= 0
    translate([20, 60, 0])
    track_strip([7, 7, 3], 2, 2, false, 0.5) {
        assert($strip_ncols == 3);
        assert($strip_nrows == 3);
    }
    
    // With stagger the holes should touch too, but not
    // overlap.
    translate([40, 60, 0])
    track_strip([7, 7, 3], 2, 2, true, 0.5) {
        assert($strip_ncols == 3);
        assert($strip_nrows == 3);
    }
    
    
    
    // === LONG THIN FORMS ===
    // 2 x 30
    translate([0, 70, 0])
    track_strip([60, 4, 3], 1, 2, true, 0.5) {
        assert($strip_ncols == 30);
        assert($strip_nrows == 2); // XX
    }
    
    // 2 x 30
    translate([0, 80, 0])
    track_strip([60, 4, 3], 1, 2, false, 0.5) {
        assert($strip_ncols == 30);
        assert($strip_nrows == 2); //XX
    }
}

if (run_tests) {
    test();
}
else
{
    track_strip([beam_length_1, beam_height, beam_thickness],
                hole_diameter, hole_pitch, hole_stagger,
                beam_margins);
}