//
//  SnapshotDisplay.h
//  AVC Controller
//
//  Created by Kirk Roerig on 2/28/16.
//  Copyright © 2016 PossomGames. All rights reserved.
//

#import <MapKit/MapKit.h>
#import "system.h"

@interface SnapshotDisplay : MKMapView <MKMapViewDelegate, CLLocationManagerDelegate, MKAnnotation>

@property (nonatomic) sysSnap_t snapshot;

@end
