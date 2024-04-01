//
// Sample code for the OAuth 2.0 device authorization grant flow using the C++ REST SDK (cpprestsdk).
//
// Microsoft identity platform and the OAuth 2.0 device authorization grant flow
// https://learn.microsoft.com/en-us/entra/identity-platform/v2-oauth2-device-code
// 
// C++ REST SDK
// https://github.com/microsoft/cpprestsdk
//

#include <cpprest/http_client.h>  // HTTP client
#include <cpprest/filestream.h>   // File stream support
#include <cpprest/json.h>         // JSON library
#include <windows.h>              // For Sleep()

using namespace utility;               // Common utilities like string conversions
using namespace concurrency::streams;  // Asynchronous streams
using namespace web;                   // Common features like URIs.
using namespace web::http;             // Common HTTP functionality
using namespace web::http::client;     // HTTP client features
using namespace web::json;             // JSON library

//
// Print HTTP request
//
void PrintRequest(const http_client& client, const http_request& request)
{
    ucout << "-------- HTTP REQUEST --------" << std::endl;
    ucout << client.base_uri().to_string() << std::endl;
    ucout << request.to_string() << std::endl;
}

//
// Print HTTP response
//
void PrintResponse(const http_response& response, const json::value& responseJson)
{
    ucout << "-------- HTTP RESPONSE --------" << std::endl;
    ucout << response.to_string();
    ucout << responseJson.serialize().c_str() << std::endl;
}

//
// Device authorization
//
http_response GetDeviceAuthorizationResponse(const uri& authUri, const utility::string_t& clientId, const utility::string_t& scope)
{
    // Create an HTTP client.
    http_client client(authUri);

    // Create a request body.
    utility::stringstream_t requestBodyStream;
    requestBodyStream << U("client_id=") << clientId << U("&scope=") << scope;
    const utf8string requestBody = utility::conversions::to_utf8string(requestBodyStream.str());

    // Create a request.
    http_request request(methods::POST);
    request.set_request_uri(uri_builder(U("/devicecode")).to_string());
    request.set_body(requestBody, "application/x-www-form-urlencoded");
    PrintRequest(client, request);  // Display the request.

    // Send a request and recieve a response.
    return client.request(request).get();
}

json::value InvokeDeviceAuthorization(const uri& authUri, const utility::string_t& clientId)
{
    // Send a request and recieve a response.
    const utility::string_t scope = web::uri::encode_data_string(U("https://management.azure.com/.default"));
    const http_response response = GetDeviceAuthorizationResponse(authUri, clientId, scope);

    // NOTE: It looks that JSON can be extract from the response only once. If you extract JSON from response multiple times, it will throw an exception.
    const json::value responseJson = response.extract_json().get();
    PrintResponse(response, responseJson);  // Display the response.
    return responseJson;
}

//
// User authentication
//
http_response GetUserAuthenticationResponse(const uri& authUri, const utility::string_t& clientId, const utility::string_t& deviceCode)
{
    // Create an HTTP client.
    http_client client(authUri);

    // Create a request body.
    utility::stringstream_t requestBodyStream;
    requestBodyStream << U("grant_type=urn:ietf:params:oauth:grant-type:device_code") << "&client_id=" << clientId << U("&device_code=") << deviceCode;
    const utf8string requestBody = utility::conversions::to_utf8string(requestBodyStream.str());

    // Create a request.
    http_request request(methods::POST);
    request.set_request_uri(uri_builder(U("/token")).to_string());
    request.set_body(requestBody, "application/x-www-form-urlencoded");
    PrintRequest(client, request);  // Display the request.

    // Send a request and recieve a response.
    return client.request(request).get();
}

json::value InvokeUserAuthentication(const uri& authUri, const utility::string_t& clientId, const utility::string_t& deviceCode, const int interval)
{
    while (true)
    {
        // Send a request and recieve a response.
        http_response response = GetUserAuthenticationResponse(authUri, clientId, deviceCode);

        // NOTE: It looks that JSON can be extract from the response only once. If you extract JSON from response multiple times, it will throw an exception.
        json::value responseJson = response.extract_json().get();
        PrintResponse(response, responseJson);  // Display the response.

        status_code statusCode = response.status_code();
        if (statusCode == status_codes::OK)
        {
            // Authentication succeeded. We got an access token.
            return responseJson;
        }
        else
        {
            const utility::string_t& error = responseJson.at(U("error")).as_string();
            const bool isAuthorizationPending = statusCode == status_codes::BadRequest && (error.compare(U("authorization_pending")) == 0);
            if (isAuthorizationPending)
            {
                // Polling the /token endpoint every interval seconds while "authorization_pending" returns as error.
                Sleep(interval * 1000);
            }
            else
            {
                // Authentication failed for some reason.
                return responseJson;
            }
        }
    }
}

//
// Acquire access token
//
utility::string_t AcquireToken(const uri& authUri, const utility::string_t& clientId)
{
    // Device authorization
    const json::value deviceAuthZResult = InvokeDeviceAuthorization(authUri, clientId);

    // Retrieve key information from the result.
    const utility::string_t userCode = deviceAuthZResult.at(U("user_code")).as_string();
    const utility::string_t message = deviceAuthZResult.at(U("message")).as_string();
    const utility::string_t deviceCode = deviceAuthZResult.at(U("device_code")).as_string();
    const int interval = deviceAuthZResult.at(U("interval")).as_integer();

    ucout << "****************\n";
    ucout << "user_code: " << userCode << std::endl;
    ucout << "message: " << message << std::endl;
    ucout << "device_code: " << deviceCode << std::endl;
    ucout << "interval: " << interval << std::endl;

    // User authentication
    const json::value userAuthNResult = InvokeUserAuthentication(authUri, clientId, deviceCode, interval);
    if (userAuthNResult.has_field(U("access_token")))
    {
        // Retrieve an access token from the result.
        return userAuthNResult.at(U("access_token")).as_string();
    }
    else
    {
        const utility::string_t& error = userAuthNResult.at(U("error")).as_string();
        ucout << "Authentication failed with " << error << std::endl;
        return U("");  // Could not acquire token.
    }
}

//
// Get URI for authorization and authentication.
//
uri GetAuthUri(const utility::string_t& tenant)
{
    const utility::string_t authorityBaseUri = U("https://login.microsoftonline.com");
    const uri authority = uri_builder(authorityBaseUri).append_path(tenant).to_uri();
    return uri_builder(authority).append_path(U("/oauth2/v2.0")).to_uri();
}

//
// Sample Azure REST API
//
// Resource Groups - List
// https://learn.microsoft.com/en-us/rest/api/resources/resource-groups/list
//
http_response ListResourceGroups(const utility::string_t& subscriptionId, const utility::string_t& apiVersion, const utility::string_t& accessToken)
{
    // Create an HTTP client.
    const utility::string_t arm_base_uri = U("https://management.azure.com");
    const uri request_uri = uri_builder(arm_base_uri)
        .append_path(U("subscriptions"))
        .append_path(subscriptionId)
        .append_path(U("resourcegroups"))
        .append_query(U("api-version"), apiVersion)
        .to_uri();
    http_client client(request_uri);

    // Create a request.
    http_request request(methods::GET);
    request.headers().add(U("Content-Type"), U("application/json"));
    request.headers().add(U("Authorization"), U("Bearer ") + accessToken);
    PrintRequest(client, request); // Display the request.

    // Send a request and recieve a response.
    return client.request(request).get();
}

//
//
//
int wmain(int argc, wchar_t* args[])
{
    if (argc < 4)
    {
        ucout << "Usage: cpprestsample.exe <tenant> <client_id> <subscription_id>" << std::endl;
		return -1;
    }

    const utility::string_t tenant = args[1];          // Tenant ID (GUID) or tenant domain
    const utility::string_t clientId = args[2];        // Application (client) ID
    const utility::string_t subscriptionId = args[3];

    // Acquire access token from Microsoft Entra ID.
    const uri authUri = GetAuthUri(tenant);
    const utility::string_t accessToken = AcquireToken(authUri, clientId);
    if (accessToken.length() == 0)
    {
        return -1;  // Could not acquire token.
    }

    // Call sample Azure REST API.
    http_response armRestApiResponse = ListResourceGroups(subscriptionId, U("2021-04-01"), accessToken);
    json::value armRestApiResponseJson = armRestApiResponse.extract_json().get();
    PrintResponse(armRestApiResponse, armRestApiResponseJson);  // Display the response.

    return 0;
}
