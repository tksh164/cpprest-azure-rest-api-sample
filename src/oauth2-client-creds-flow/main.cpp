//
// Sample code for the OAuth 2.0 client credentials flow using the C++ REST SDK (cpprestsdk).
//
// Microsoft identity platform and the OAuth 2.0 client credentials flow
// https://learn.microsoft.com/en-us/entra/identity-platform/v2-oauth2-client-creds-grant-flow
// 
// C++ REST SDK
// https://github.com/microsoft/cpprestsdk
//

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/json.h>

using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;

pplx::task<json::value> GetAccessToken(const utility::string_t& tenantId, const utility::string_t& clientId, const utility::string_t& clientSecret)
{
    return pplx::create_task([tenantId, clientId, clientSecret]
    {
        http_client client(U("https://login.microsoftonline.com/") + tenantId + U("/oauth2/v2.0/token"));

        // Create a request.
        http_request request(methods::POST);
        request.headers().add(U("Content-Type"), U("application/x-www-form-urlencoded"));

        // Set the request body.
        utility::string_t body = U("grant_type=client_credentials&client_id=") + clientId + U("&client_secret=") + clientSecret + U("&resource=https%3A%2F%2Fmanagement.azure.com%2F");
        request.set_body(body, U("application/x-www-form-urlencoded"));

        // Sen the request and recieve response.
        return client.request(request)
            .then([](http_response response)
            {
                // Display the response.
                ucout << "Status: " << response.status_code() << std::endl;
                return response.extract_json();
            })
            .then([](json::value jsonValue)
            {
                // Display the response body.
                ucout << "Response: " << jsonValue.serialize().c_str() << std::endl;
                return jsonValue;
            });
    });
}

//
// Sample Azure REST API
//
// Virtual Machines - List
// https://learn.microsoft.com/en-us/rest/api/compute/virtual-machines/list
//
pplx::task<void> GetVMInfo(const utility::string_t& accessToken, const utility::string_t& subscriptionId, const utility::string_t& resourceGroupName)
{
    return pplx::create_task([accessToken, subscriptionId, resourceGroupName]
    {
        http_client client(U("https://management.azure.com/subscriptions/") + subscriptionId + U("/resourceGroups/") + resourceGroupName + U("/providers/Microsoft.Compute/virtualMachines?api-version=2024-03-01"));

        // Create a request.
        http_request request(methods::GET);
        request.headers().add(U("Authorization"), U("Bearer ") + accessToken);

        // Sen the request and recieve response.
        return client.request(request)
            .then([](http_response response)
            {
                // Display the response.
                ucout << "Status: " << response.status_code() << std::endl;
                return response.extract_string();
            })
            .then([](std::wstring body)
            {
                ucout << "Body: " << body << std::endl;
            });
    });
}

int wmain(int argc, wchar_t* args[])
{
    // Configuration for Microsoft Entra ID
    const utility::string_t tenantId = U("TENANT_ID");                     // Tenant ID (GUID) or tenant domain
    const utility::string_t clientId = U("SP_APP_ID");                     // Application (client) ID
    const utility::string_t clientSecret = U("SP_SECRET");                 // Client secret

    // Configuration for VM info.
    const utility::string_t subscriptionId = U("SUBSCRIPTION_ID");         // Azure subscription ID
    const utility::string_t resourceGroupName = U("RESOURCE_GROUP_NAME");  // Target resource group name

    GetAccessToken(tenantId, clientId, clientSecret)
        .then([subscriptionId, resourceGroupName](json::value jsonValue)
        {
            utility::string_t accessToken = jsonValue[U("access_token")].as_string();
            return GetVMInfo(accessToken, subscriptionId, resourceGroupName);
        })
        .wait();

    return 0;
}
