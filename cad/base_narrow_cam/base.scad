include <bolts/BOLTS.scad>
$fn=30;

WIDTH = 98.0;
LENGTH = 75;
HEIGHT=3.5;
HOLDER_HEIGHT = 1.7;
HOLDER_WIDTH = 5.5;


BOARD_HOLE_WIDTH=55;
BOARD_HOLE_LENGTH=65;
BOARD_MOUNTING_HEIGHT=5;

module board_holder(height) {
    union() {
        difference() {
            cube([10, 10, height]);
            translate([5, 5, -0.5]) {
                cylinder(height+1, d=3.2);
                cylinder(height, d=3.4);
            }
            translate([5, 5, -0.01]) 
            scale([1.1, 1.1, 1])
                ISO4032("M3",part_mode="diff");    
        }
    }
}


union() {

    difference() {
        union() {
            cube([WIDTH, LENGTH, HOLDER_HEIGHT]);
            translate([HOLDER_WIDTH, 0, HOLDER_HEIGHT/2])
            cube([WIDTH - (2*HOLDER_WIDTH), LENGTH, HEIGHT]);
        }
        translate([(WIDTH-BOARD_HOLE_WIDTH-10)/2, 0, 0]) {
            translate([0, 0, -0.05])
            cube(10, 10, BOARD_MOUNTING_HEIGHT+HEIGHT+0.1);
            translate([BOARD_HOLE_WIDTH, 0, -0.05])
            cube(10, 10, BOARD_MOUNTING_HEIGHT+HEIGHT+0.1);
            translate([0, BOARD_HOLE_LENGTH, -0.05])
            cube(10, 10, BOARD_MOUNTING_HEIGHT+HEIGHT+0.1);
            translate([BOARD_HOLE_WIDTH, BOARD_HOLE_LENGTH, -0.05])
            cube(10, 10, BOARD_MOUNTING_HEIGHT+HEIGHT+0.1);
        }
        
        translate([(WIDTH-BOARD_HOLE_WIDTH-9.5)/2 - 6, 30, -0.1]) {
            cylinder(10, d=3.2);            
        }
        translate([WIDTH-((WIDTH-BOARD_HOLE_WIDTH-9.5)/2 - 6), 30, -0.1]) {
            cylinder(10, d=3.2);            
        }
        translate([(WIDTH-BOARD_HOLE_WIDTH-9.5)/2 - 6, 40, -0.1]) {
            cylinder(10, d=3.2);            
        }
        translate([WIDTH-((WIDTH-BOARD_HOLE_WIDTH-9.5)/2 - 6), 40, -0.1]) {
            cylinder(10, d=3.2);            
        }
        translate([(WIDTH-BOARD_HOLE_WIDTH-9.5)/2 - 6, 50, -0.1]) {
            cylinder(10, d=3.2);            
        }
        translate([WIDTH-((WIDTH-BOARD_HOLE_WIDTH-9.5)/2 - 6), 50, -0.1]) {
            cylinder(10, d=3.2);            
        }
        translate([(WIDTH-BOARD_HOLE_WIDTH-9.5)/2 - 6, 60, -0.1]) {
            cylinder(10, d=3.2);            
        }
        translate([WIDTH-((WIDTH-BOARD_HOLE_WIDTH-9.5)/2 - 6), 60, -0.1]) {
            cylinder(10, d=3.2);            
        }

    }
                
    translate([(WIDTH-BOARD_HOLE_WIDTH-10)/2, 0, 0]) {
        translate([0, 0, -0.05])
        board_holder(BOARD_MOUNTING_HEIGHT+HEIGHT+0.1);
        translate([BOARD_HOLE_WIDTH, 0, -0.05])
        board_holder(BOARD_MOUNTING_HEIGHT+HEIGHT+0.1);
        translate([0, BOARD_HOLE_LENGTH, -0.05])
        board_holder(BOARD_MOUNTING_HEIGHT+HEIGHT+0.1);
        translate([BOARD_HOLE_WIDTH, BOARD_HOLE_LENGTH, -0.05])
        board_holder(BOARD_MOUNTING_HEIGHT+HEIGHT+0.1);
    }
}

