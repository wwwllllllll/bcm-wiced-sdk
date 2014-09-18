--------------------------------------------------------------------------------
WICED Resources Directory - README
--------------------------------------------------------------------------------

The Resources directory contains :
  - WICED webserver components such as web pages, images, scripts, styles
  - WLAN firmware
  - Security certificates

All of these components, except possibly security certificates, are put into the
flash file system. Security certificates may be put into the DCT.

Webserver components are used by the WICED webserver.
To add a web resource (html page, script, css style etc) for a WICED application
called MY_APP_NAME, follow these steps:

1. Add your file(s) in the <WICED-SDK>/resources/apps/<MY_APP_NAME> directory

2. Edit <WICED-SDK>/Apps/<subdir>/<APP_NAME>/<MY_APP_NAME>.mk and add your file to the list of Resources
   eg. $(NAME)_RESOURCES := images/brcmlogo.png \
                            apps/<MY_APP_NAME>/my_web_page.html

3. In your application, add an entry to the webpage database for your resource file.
   For example, if your filename is 'my_web_page.html', the entry in the
   application webpage database will be something like the following:

   {
        .url = "/apps/<MY_APP_NAME>/my_web_page.html",
        .mime_type = "text/html",
        .url_content_type = WICED_STATIC_URL_CONTENT,
        .url_content.static_data = {resource_apps_DIR_<MY_APP_NAME>_DIR_my_web_page_html, sizeof(resource_apps_DIR_<MY_APP_NAME>_DIR_my_web_page_html)-1},
   },

Example Usage
  It is instructive to review how resources are used by working through
  one of the various example applications that use resources. Apps such
  as the snip/ping_webserver or demo/temp_control apps provide good
  examples.