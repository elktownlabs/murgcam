$fn=30;


difference() {
    linear_extrude(height=20)
        square([34, 7]);
    translate([3.5, 3.5, 0])
        cylinder(d=3.2, h=20);
    translate([34-3.5, 3.5, 0])
        cylinder(d=3.2, h=20);
    
    translate([7, 0, 0])
    linear_extrude(height=6)
    square(20,7);
    
    translate([7, 4.5, 0])
    linear_extrude(height=20)
    square([20,2.5]);

    translate([7, 0, 0])
    linear_extrude(height=20)
    square([20,2.5]);


}
