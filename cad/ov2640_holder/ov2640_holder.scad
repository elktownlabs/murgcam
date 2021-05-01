$fn=30;

union() {
    difference() {
        linear_extrude(height=20)
            square([34, 7]);
        translate([3.5, 3.5, 0])
            cylinder(d=3.2, h=20);
        translate([34-3.5, 3.5, 0])
            cylinder(d=3.2, h=20);
        
        translate([7, 0, 0])
        linear_extrude(height=20)
        square(20,7);
    }
    
    translate([5, 6, 7])
    difference() {    
        linear_extrude(height=13)
        square([24, 15]);

        translate([2, -2, 0])
        linear_extrude(height=13)
        square([20, 15]);
        
    }
}
