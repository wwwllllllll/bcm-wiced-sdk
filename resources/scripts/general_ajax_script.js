
function insert_and_eval( elemid, content )
{
  var elem = document.getElementById( elemid );
  elem.innerHTML = content;
  var scripts = elem.getElementsByTagName("script");
  for(var i=0; i < scripts.length; i++)
  {
    eval(scripts[i].innerHTML || scripts[i].text); // .text is necessary for IE.
  }
}

var AJAX_FINISHED           =  0;
var AJAX_NO_BROWSER_SUPPORT = -1;
var AJAX_STARTING           = -2;
var AJAX_PARTIAL_PROGRESS   = -3;
var AJAX_FAILED             = -4;

function do_ajax( ajax_url, callback_func )
{
  req = null;
  if (window.XMLHttpRequest)
  {
    req = new XMLHttpRequest();
    if (req.overrideMimeType)
    {
      req.overrideMimeType('text/xml');
    }
  }
  else if (window.ActiveXObject)
  {
    try {
      req = new ActiveXObject("Msxml2.XMLHTTP");
    } catch (e)
    {
      try {
          req = new ActiveXObject("Microsoft.XMLHTTP");
      } catch (e) {
        if ( callback_func != null ) callback_func( AJAX_NO_BROWSER_SUPPORT, null );
        return;
      }
    }
  }
  try {
    req.onprogress = function( e )
    {
        if ( callback_func != null ) callback_func( AJAX_PARTIAL_PROGRESS, req.responseText );
    }
  } catch (e) { if ( callback_func != null ) callback_func( AJAX_PARTIAL_PROGRESS, null ); }
  req.onreadystatechange = function()
  {
    if(req.readyState == 4)
    {
      if ((req.status == 200) 
//          || ( req.status == 0 )   // For testing local files in chrome with chrome.exe --allow-file-access-from-files
          )
      {
        if ( callback_func != null ) callback_func( AJAX_FINISHED, req.responseText );
      }
      else
      {
        if ( callback_func != null ) callback_func( AJAX_FAILED, null );
      }
    }
  };
  req.open("GET", ajax_url, true);
  req.send(null);
  if ( callback_func != null ) callback_func( AJAX_STARTING, null );
}

