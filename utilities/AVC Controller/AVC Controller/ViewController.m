//
//  ViewController.m
//  AVC Controller
//
//  Created by Kirk Roerig on 12/29/15.
//  Copyright © 2015 PossomGames. All rights reserved.
//

#import "ViewController.h"
#import "clientAddress.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

@interface ViewController ()

@property (weak, nonatomic) IBOutlet ThumbStickControl *throttleStick;
@property (weak, nonatomic) IBOutlet ThumbStickControl *steerStick;
@property (weak, nonatomic) IBOutlet UITextField *ipAddressText;
@property (weak, nonatomic) IBOutlet UIActivityIndicatorView *resolvingIndicator;

@end

typedef struct{
    uint32_t id;
    uint8_t  throttle;
    uint8_t  steering;
} rcMessage_t;

struct sockaddr_in* HOST_ADDRESS;
rcMessage_t STATE = { 0, 50, 50 };

@implementation ViewController

void transmit(){
    static int sock;

    
    if(!sock){
        sock = socket(AF_INET, SOCK_DGRAM, 0);
    }
    
    sendto(
        sock,
        &STATE,
        sizeof(rcMessage_t),
        0,
        (const struct sockaddr*)&HOST_ADDRESS,
        sizeof(HOST_ADDRESS)
    );
}

- (void)viewDidLayoutSubviews
{
    [self.throttleStick reset];
    [self.steerStick reset];
    
    self.throttleStick.xAxisDisabled = YES;
    self.steerStick.yAxisDisabled    = YES;
    
    [self.throttleStick setRangeForAxis:1 withMin:75 andMax:25];
    [self.steerStick setRangeForAxis:0 withMin:25 andMax:75];
    
    self.throttleStick.delegate = self;
    self.steerStick.delegate    = self;
    
    [self.view addGestureRecognizer:[[UITapGestureRecognizer alloc]
                                     initWithTarget:self
                                     action:@selector(dismiss)]];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    
    [self.resolvingIndicator stopAnimating];
}

- (void)didThumbMoveWithValues:(CGPoint)values asSender:(ThumbStickControl *)sender
{
    if(sender == self.throttleStick){
        STATE.throttle = values.y;
    }
    
    if(sender == self.steerStick){
        STATE.steering = values.x;
    }
    
    transmit();
}

- (IBAction)didChangeIpAddress:(id)sender {
    
    [self.resolvingIndicator startAnimating];
    
    dispatch_async(dispatch_get_main_queue(), ^{
        struct hostent* he = NULL;
        const char* hostname = [self.ipAddressText.text UTF8String];
        
        if(!(he = gethostbyname(hostname))){
            [self.resolvingIndicator stopAnimating];
            
            UIAlertController* alert = [UIAlertController alertControllerWithTitle:@"Error"
                                                                           message:[NSString stringWithFormat:@"Couldn't resolve host '%s'", hostname]
                                                                    preferredStyle:UIAlertControllerStyleAlert];
            [alert addAction:[UIAlertAction actionWithTitle:@"OK"
                                                      style:UIAlertActionStyleCancel
                                                    handler:^(UIAlertAction * _Nonnull action) { }]];
            [self presentViewController:alert animated:YES completion:^{ }];
            
            free(HOST_ADDRESS);
            HOST_ADDRESS = NULL;
            
            return;
        }

        if(!HOST_ADDRESS){
            HOST_ADDRESS = malloc(sizeof(struct sockaddr_in));
        }
            
        HOST_ADDRESS->sin_family = AF_INET;
        HOST_ADDRESS->sin_port   = htons(1338);
        unsigned char* addr = (unsigned char*)he->h_addr_list[0];
        memcpy((void*)&(HOST_ADDRESS->sin_addr), addr, he->h_length);
        
        self.ipAddressText.text = [NSString stringWithFormat:@"%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]];
        [self.resolvingIndicator stopAnimating];
    });
    

    
}

- (void)dismiss {
    [self.ipAddressText resignFirstResponder];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
