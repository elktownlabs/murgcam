include <bolts/BOLTS.scad>

$fn=30;

difference() {

    union() {
        linear_extrude(height=2.0)
            square([105,80]);
        translate([0, 0, 2])        
        linear_extrude(height=2.5)
            square([3, 80]);
        translate([102, 0, 2])        
        linear_extrude(height=2.5)
            square([3, 80]);
       
       
        translate([20, 0, 2.0]) {
            difference() {
                translate([0, 0, 00]) 
                linear_extrude(height=5)
                square([65,75]);
                translate([10, -0, 00]) 
                linear_extrude(height=5)
                square([45,75]);
                translate([0, 10, 00]) 
                linear_extrude(height=5)
                square([65,55]);
            }
        }
    }

    translate([20, 0, 0]) {
        translate([5, 5, 0])
        cylinder(10, d=3.2);
        translate([60, 5, 0])
        cylinder(10, d=3.2);
        translate([5, 70, 0])
        cylinder(10, d=3.2);
        translate([60, 70, 0])
        cylinder(10, d=3.2);
        translate([5, 5, -0.01]) ISO4032("M3",part_mode="diff");    
        translate([60, 5, -0.01]) ISO4032("M3",part_mode="diff");    
        translate([5, 70, -0.01]) ISO4032("M3",part_mode="diff");    
        translate([60, 70, -0.01]) ISO4032("M3",part_mode="diff");    
      
    }

}
