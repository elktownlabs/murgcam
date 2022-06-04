include <bolts/BOLTS.scad>

$fn=30;

difference() {

    union() {
        linear_extrude(height=3.0)
            square([105,110]);
        translate([0, 0, 2])        
        linear_extrude(height=2.5)
            square([5, 110]);
        translate([100, 0, 2])        
        linear_extrude(height=2.5)
            square([5, 110]);
        
        translate([4.5, 0, 0])
        linear_extrude(height=10)
          square([96, 110]);        
    }

        translate([7.5, 3, 3.0])
        linear_extrude(height=10)
          square([90, 104]);        

        translate([2.5, 15, 0.0])
        linear_extrude(height=5)
          square([2, 10]);        

        translate([2.5, 85, 0.0])
        linear_extrude(height=5)
          square([2, 10]);        

        translate([100.5, 15, 0.0])
        linear_extrude(height=5)
          square([2, 10]);        

        translate([100.5, 85, 0.0])
        linear_extrude(height=5)
          square([2, 10]);

}
