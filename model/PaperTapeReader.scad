/**
 * 1" punched paper tape reader model.
 *
 * 20150928 - Gavin Stewart.
 */

/**
 * Tape dimension constants.
 */
tapeWidth = 25.4;				// mm (1.0")
holePitch = 2.54;				// mm (0.1")
dataHoleRadius = 1.83 / 2 + 0.05;	// mm (0.072" diameter) + 0.05 extra
feedHoleRadius = 1.17 / 2 + 0.10;	// mm (0.046" diameter) + 0.10 extra

/**
 * Reader dimesion constants.
 */
shoulderWidth = 10;
baseThickness = 4 + 1;
baseLength = 70;
gateLength = 25;
guideLength = 50;
guideThickness = 0.8;
cornerRadius = 3;				// Rounded corners on parts.
partSpacing = 2;				// There are three stacked parts.
screwHoleRadius = 3 / 2;

/**
 * Calculated constants.
 */
halfShoulderWidth = shoulderWidth / 2;
baseWidth = tapeWidth + (shoulderWidth * 2);

/**
 * Set default number of fragments for OpenSCAD curves.
 */
$fn = 32;

/**
 * Correction for differenced shapes in mm.
 * Fudge to make sure that holes/cuts/paths are big enough for screws, etc.
 */
differenceCorrection = 0.15;

/**
 * LED / phototransistor pathway.
 */
module ledPathway (length) {   
	bodyRadius = 1.5 + (differenceCorrection / 2);	
	lipRadius = 1.7 + (differenceCorrection / 2);
   height = 3.7; 		// Height of led body (after shoulder) to tip.
 
	translate([0, 0, bodyRadius]) {
		sphere(r = bodyRadius);
		cylinder(h = height - bodyRadius + 0.1, r = bodyRadius);
	   translate([0, 0, height - bodyRadius])
			cylinder(h = length, r = lipRadius);
   }
}

/**
 * Generic hole pathway with inside diameter correction.
 */
module holePathway (radius, length) {
	radiusCorrected = radius + (differenceCorrection / 2);
	cylinder(h = length, r = radius);
}

/**
 * An ledPathway() with a holePathway() in front of it.
 */
module ledPathwayAndHole (ledLength, holeRadius, holeLength) {
	ledPathway(ledLength);
	translate([0, 0, -ledLength])
 		holePathway(holeRadius, holeLength + 1);
}

/**
 * Rounded cube only on Z axis. Uses cornerRadius constant.
 */
module roundedCubeZ (size) {
	hull() {
		translate([cornerRadius, cornerRadius, 0])
			cylinder(r = cornerRadius, h = size[2]);
		translate([size[0] - cornerRadius, cornerRadius, 0])
			cylinder(r = cornerRadius, h = size[2]);
		translate([cornerRadius, size[1] - cornerRadius, 0])
			cylinder(r = cornerRadius, h = size[2]);
		translate([size[0] - cornerRadius, size[1] - cornerRadius, 0])
			cylinder(r = cornerRadius, h = size[2]);
	}
}

/**
 * Paper tape track body.
 */
module trackBody () {
	paperPathWidth = tapeWidth + differenceCorrection;
	led9X = baseLength / 2 + holePitch;
	led9Y = shoulderWidth + (differenceCorrection / 2) + holePitch;
	edgeChannelWidth = 1;

	difference() {
		// base
		roundedCubeZ([baseLength, baseWidth, baseThickness]);

		// outer edge channels of tape pathway for alignment of edge side assembly.
		translate([-0.1, shoulderWidth - edgeChannelWidth, baseThickness - edgeChannelWidth])
			cube([baseLength + 0.2, edgeChannelWidth, edgeChannelWidth + 0.1]);
		translate([-0.1, shoulderWidth + paperPathWidth, baseThickness - edgeChannelWidth])
			cube([baseLength + 0.2, edgeChannelWidth, edgeChannelWidth + 0.1]);

		// 9 LED pathways, starting at 9.
		translate([led9X - (holePitch * 0), led9Y + (holePitch * 0), baseThickness - 1]) 
			rotate([0, 180, 0]) 
				ledPathwayAndHole(baseThickness, dataHoleRadius, baseThickness);
		translate([led9X - (holePitch * 2), led9Y + (holePitch * 1), baseThickness - 1]) 
			rotate([0, 180, 0]) 
				ledPathwayAndHole(baseThickness, dataHoleRadius, baseThickness);
		translate([led9X - (holePitch * 0), led9Y + (holePitch * 2), baseThickness - 1]) 
			rotate([0, 180, 0]) 
				ledPathwayAndHole(baseThickness, dataHoleRadius, baseThickness);
		translate([led9X - (holePitch * 2), led9Y + (holePitch * 3), baseThickness - 1]) 
			rotate([0, 180, 0]) 
				ledPathwayAndHole(baseThickness, dataHoleRadius, baseThickness);
		translate([led9X - (holePitch * 0), led9Y + (holePitch * 4), baseThickness - 1]) 
			rotate([0, 180, 0]) 
				ledPathwayAndHole(baseThickness, dataHoleRadius, baseThickness);
		translate([led9X - (holePitch * 2), led9Y + (holePitch * 5), baseThickness - 1]) 
			rotate([0, 180, 0]) 
				ledPathwayAndHole(baseThickness, feedHoleRadius, baseThickness);
		translate([led9X - (holePitch * 0), led9Y + (holePitch * 6), baseThickness - 1]) 
			rotate([0, 180, 0]) 
				ledPathwayAndHole(baseThickness, dataHoleRadius, baseThickness);
		translate([led9X - (holePitch * 2), led9Y + (holePitch * 7), baseThickness - 1]) 
			rotate([0, 180, 0]) 
				ledPathwayAndHole(baseThickness, dataHoleRadius, baseThickness);
		translate([led9X - (holePitch * 0), led9Y + (holePitch * 8), baseThickness - 1]) 
			rotate([0, 180, 0]) 
				ledPathwayAndHole(baseThickness, dataHoleRadius, baseThickness);

		// Corner screw holes (for attaching rubber feet, or mounting).
		translate([halfShoulderWidth, halfShoulderWidth, -0.1])
			holePathway(screwHoleRadius, baseThickness + 0.2);
		translate([baseLength-halfShoulderWidth, halfShoulderWidth, -0.1])
			holePathway(screwHoleRadius, baseThickness + 0.2);
		translate([halfShoulderWidth, baseWidth - halfShoulderWidth, -0.1])
			holePathway(screwHoleRadius, baseThickness + 0.2);
		translate([baseLength-halfShoulderWidth, baseWidth - halfShoulderWidth, -0.1])
			holePathway(screwHoleRadius, baseThickness + 0.2);
		
	}
}

module topGate () {
	paperPathWidth = tapeWidth + differenceCorrection;
	led9X = gateLength / 2 + holePitch;
	led9Y = shoulderWidth + (differenceCorrection / 2) + holePitch;
	
	// Centre over trackBody()
	translate([(baseLength / 2) - (gateLength / 2), 0, baseThickness + guideThickness + partSpacing * 2]) {

	difference() {
		roundedCubeZ([gateLength, baseWidth, baseThickness]);

		// 9 LED pathways, starting at 9.
		translate([led9X - (holePitch * 0), led9Y + (holePitch * 0), 1]) 	
				ledPathwayAndHole(baseThickness, dataHoleRadius, baseThickness);
		translate([led9X - (holePitch * 2), led9Y + (holePitch * 1), 1]) 
				ledPathwayAndHole(baseThickness, dataHoleRadius, baseThickness);
		translate([led9X - (holePitch * 0), led9Y + (holePitch * 2), 1]) 
				ledPathwayAndHole(baseThickness, dataHoleRadius, baseThickness);
		translate([led9X - (holePitch * 2), led9Y + (holePitch * 3), 1]) 
				ledPathwayAndHole(baseThickness, dataHoleRadius, baseThickness);
		translate([led9X - (holePitch * 0), led9Y + (holePitch * 4), 1]) 
				ledPathwayAndHole(baseThickness, dataHoleRadius, baseThickness);
		translate([led9X - (holePitch * 2), led9Y + (holePitch * 5), 1])  
				ledPathwayAndHole(baseThickness, feedHoleRadius, baseThickness);
		translate([led9X - (holePitch * 0), led9Y + (holePitch * 6), 1])  
				ledPathwayAndHole(baseThickness, dataHoleRadius, baseThickness);
		translate([led9X - (holePitch * 2), led9Y + (holePitch * 7), 1]) 
				ledPathwayAndHole(baseThickness, dataHoleRadius, baseThickness);
		translate([led9X - (holePitch * 0), led9Y + (holePitch * 8), 1])  
				ledPathwayAndHole(baseThickness, dataHoleRadius, baseThickness);
	}

	}
}

module tapeGuide () {

	// Centre over trackBody()
	translate([(baseLength / 2) - (guideLength / 2), 0, baseThickness + partSpacing]) {

	// One side
	translate([0, halfShoulderWidth, 0])
		hull() {
			translate([halfShoulderWidth, 0, 0])
				cylinder(r = halfShoulderWidth, h = guideThickness);
			translate([guideLength - halfShoulderWidth, 0, 0])
				cylinder(r = halfShoulderWidth, h = guideThickness);
		}

	// Other side
	translate([0, baseWidth - halfShoulderWidth, 0])
		hull() {
			translate([halfShoulderWidth, 0, 0])
				cylinder(r = halfShoulderWidth, h = guideThickness);
			translate([guideLength - halfShoulderWidth, 0, 0])
				cylinder(r = halfShoulderWidth, h = guideThickness);
		}

	}
}


// Combine parts, and cut screw holes through them.
screwBaseX = (baseLength / 2) - (gateLength / 2);
screwBaseHoleHeight = baseThickness + baseThickness + guideThickness + partSpacing * 2 + 0.2;
difference() {
	union() {
		trackBody();
		tapeGuide();
		topGate();
	}
	// Screw holes
	translate([screwBaseX + halfShoulderWidth, halfShoulderWidth, -0.1])
		holePathway(screwHoleRadius, screwBaseHoleHeight);
	translate([screwBaseX + gateLength - halfShoulderWidth, halfShoulderWidth, -0.1])
		holePathway(screwHoleRadius, screwBaseHoleHeight);
	translate([screwBaseX + halfShoulderWidth, baseWidth - halfShoulderWidth, -0.1])
		holePathway(screwHoleRadius, screwBaseHoleHeight);
	translate([screwBaseX + gateLength - halfShoulderWidth, baseWidth - halfShoulderWidth, -0.1])
		holePathway(screwHoleRadius, screwBaseHoleHeight);
}

