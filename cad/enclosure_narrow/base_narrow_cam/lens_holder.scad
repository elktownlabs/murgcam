include <bolts/BOLTS.scad>

$fn=30;

BOARD_HOLE_WIDTH=55;
COLUMN_WIDTH=9.5;
HEIGHT=40;

DIST = BOARD_HOLE_WIDTH+(2*COLUMN_WIDTH)+2;

// left left
difference() {
    union() {
        cube([COLUMN_WIDTH, 10, HEIGHT]);
        translate([(COLUMN_WIDTH-15)/2, 0, 14])
        cube([15, 10, 12]);
    }
    rotate([-90, 0, 0])
        translate([4.8, -20, -0.1]) {
            scale([1.1, 1.1, 1.6])
            ISO4032("M5",part_mode="diff");    
            cylinder(h=45, d=6.6);
        }

    translate([COLUMN_WIDTH/2, 5, 0])
    cylinder(h=15, d=3.2);
    translate([COLUMN_WIDTH/2-(5.7/2), (10-5.7)/2-3, 3])
    cube([5.7, 8.7, 2.6]);


}

// right leg
translate([DIST, 0, 0])
difference() {
    cube([COLUMN_WIDTH, 10, HEIGHT]);
    translate([COLUMN_WIDTH/2, 5, 0])
    cylinder(h=15, d=3.2);
    translate([COLUMN_WIDTH/2-(5.7/2), (10-5.7)/2-3, 3])
    cube([5.7, 8.7, 2.6]);
}

difference() {
    union() {
        translate([0, 0, HEIGHT-10])
        cube([DIST+COLUMN_WIDTH, 10, 10]);
        translate([(DIST+COLUMN_WIDTH-18)/2, 0, HEIGHT-10-4]) 
        cube([18, 10, 18]);
    }
    
    
    // ccd slot
    translate([(DIST+COLUMN_WIDTH-9.2)/2, 0, HEIGHT-9.2-((10-9.2)/2)]) {
        cube([9.2, 1.2, 9.2]);
    }
    translate([(DIST+COLUMN_WIDTH-6.2)/2, 0, 28.5]) {
        cube([6.2, 1.2, 9.2]);
    }   
   
   // mounting holes
   translate([(DIST+COLUMN_WIDTH)/2-10, 10, (HEIGHT-5)])
   rotate([90, 0, 0])
   cylinder(d=2.7, h=10); 
   translate([(DIST+COLUMN_WIDTH)/2+10, 10, (HEIGHT-5)])
   rotate([90, 0, 0])
   cylinder(d=2.7, h=10); 
    
    // mounting holes for screw heads
   translate([(DIST+COLUMN_WIDTH)/2-10, 10, (HEIGHT-5)])
   rotate([90, 0, 0])
   cylinder(d=5.5, h=6); 
   translate([(DIST+COLUMN_WIDTH)/2+10, 10, (HEIGHT-5)])
   rotate([90, 0, 0])
   cylinder(d=5.5, h=6); 

}
