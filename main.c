//
//  main.c
//  xmouse
//
//  Created by nil on 2007-08-31.
//  Copyright Nathan Laredo 2007. All rights reserved.
//

#include <Carbon/Carbon.h>
#include <stdio.h>
#include <string.h>

#ifndef kCGEventTapOptionDefault
#define kCGEventTapOptionDefault 0
#endif

static OSStatus        AppEventHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon );
static OSStatus        WindowEventHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon );
static CGEventRef      AppEventTapCallback( CGEventTapProxy proxy, CGEventType type, CGEventRef inEvent, void* inRefcon );
static WindowRef       window;
static CFMachPortRef   machport;
static CFRunLoopSourceRef cfrls;
static int             connection;
extern OSStatus CGSSetWindowAlpha(long cid, long wid, float f);

static IBNibRef        sNibRef;

//--------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    OSStatus                    err;
    EventHandlerUPP             eventhandler;
    static const EventTypeSpec    kAppEvents[] =
    {
        { kEventClassCommand, kEventCommandProcess },
	{ kEventClassWindow, kEventWindowClose }
    };

    // Create a Nib reference, passing the name of the nib file (without the .nib extension).
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference( CFSTR("main"), &sNibRef );
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib( sNibRef, CFSTR("MenuBar") );
    require_noerr( err, CantSetMenuBar );
    
    eventhandler = NewEventHandlerUPP( AppEventHandler );
    // Install our handler for common commands on the application target
    InstallApplicationEventHandler( eventhandler,
                                    GetEventTypeCount( kAppEvents ), kAppEvents,
                                    0, NULL );

    // Create a new window. A full-fledged application would do this from an AppleEvent handler
    // for kAEOpenApplication.
    err = CreateWindowFromNib( sNibRef, CFSTR("MainWindow"), &window );
    require_noerr( err, CantCreateWindow );

    // Install a command handler on the window. We don't use this handler yet, but nearly all
    // Carbon apps will need to handle commands, so this saves everyone a little typing.
    InstallWindowEventHandler( window, eventhandler,
                               GetEventTypeCount( kAppEvents ), kAppEvents,
                               window, NULL );
    
    // The window was created hidden, so show it
    ShowWindow( window );

    connection = _CGSDefaultConnection();

    // Create an event tap (This is why xmouse needs OSX 10.4 and later only)
    machport = CGEventTapCreate( kCGAnnotatedSessionEventTap,
				 kCGHeadInsertEventTap,
				 kCGEventTapOptionDefault,
				 CGEventMaskBit(kCGEventMouseMoved),
				 AppEventTapCallback,
				 NULL);

    // Create a CFRunLoopSource object for a CFMachPort object
    cfrls = CFMachPortCreateRunLoopSource( NULL, machport, 0 );
    // adds a CFRunLoopSource object to a run loop mode
    CFRunLoopAddSource( CFRunLoopGetCurrent(), cfrls, kCFRunLoopCommonModes );
    
    //CFRunLoopRun();
    // Run the event loop
    RunApplicationEventLoop();

CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
    return err;
}
//--------------------------------------------------------------------------------------------
static CGEventRef
AppEventTapCallback( CGEventTapProxy proxy, CGEventType type, CGEventRef inEvent, void* inRefcon )
{
    CGPoint where = CGEventGetLocation(inEvent), localwhere;
    CGEventTimestamp when = CGEventGetTimestamp(inEvent);
    int wid, cid;
    static int old_wid = 0, old_cid = 0;;

    CGSFindWindowByGeometry( connection, 0, 1, 0, &where, &localwhere, &wid, &cid);
    if (wid != old_wid) {
	old_wid = wid;
	old_cid = cid;
	//fprintf(stderr, "0x%08x: %9.1f %9.1f, wid=0x%08x, cid=0x%08x %08x\n",
	//	inEvent, where.x, where.y, wid, cid, FrontWindow());
	//CGSSetWindowAlpha( connection, wid, 0.75 );
	CGPostMouseEvent(where, false, 1, true);
	CGPostMouseEvent(where, false, 1, false);
    }
    // As a passive listener, just return the original event...
    return inEvent;
}

//--------------------------------------------------------------------------------------------
static OSStatus
AppEventHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon )
{
    OSStatus    result = eventNotHandledErr;
    
    switch ( GetEventClass( inEvent ) )
    {
        case kEventClassCommand:
        {
            HICommandExtended cmd;
            verify_noerr( GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof( cmd ), NULL, &cmd ) );
            
            switch ( GetEventKind( inEvent ) )
            {
                case kEventCommandProcess:
                    switch ( cmd.commandID )
                    {

                        case kHICommandPreferences:
			    ShowWindow( window );
			    result = noErr;
                            break;
                            
                        // Add your own command-handling cases here
                        
                        default:
                            break;
                    }
                    break;
            }
            break;
        }
	case kEventClassWindow:
	{
            switch ( GetEventKind( inEvent ) )
            {
		// keep preferences window around, just hide it on close...
                case kEventWindowClose:
		    HideWindow( window );
		    result = noErr;
		    break;
	    }
	    break;
	}
            
        default:
            break;
    }
    
    return result;
}
