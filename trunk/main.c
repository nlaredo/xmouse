//
//  main.c
//  xmouse
//
//  Created by nil on 2007-08-31.
//  Copyright Nathan Laredo 2007. All rights reserved.
//

#include <Carbon/Carbon.h>

static OSStatus        AppEventHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon );
static OSStatus        WindowEventHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon );
static WindowRef       window;

static IBNibRef        sNibRef;

//--------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    OSStatus                    err;
    EventHandlerUPP             eventhandler;
    static const EventTypeSpec    kAppEvents[] =
    {
        { kEventClassCommand, kEventCommandProcess },
	{ kEventClassMouse, kEventMouseMoved },
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

    // Install same event handler for monitoring events from all processes
    InstallEventHandler( GetEventMonitorTarget(), eventhandler,
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
    
    // Run the event loop
    RunApplicationEventLoop();

CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
    return err;
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
