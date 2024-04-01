# Azure REST API samples using C++ REST SDK (cpprestsdk)

This repo contains Azure REST API samples using C++ REST SDK (cpprestsdk).

- [OAuth 2.0 client credentials flow sample](#oauth-20-client-credentials-flow-sample)
- [OAuth 2.0 device authorization grant flow sample](#oauth-20-device-authorization-grant-flow-sample)

## OAuth 2.0 client credentials flow sample

This sample do the following:

1. Get an access token from Microsoft Entra ID with your service principal using the [OAuth 2.0 client credentials flow](https://learn.microsoft.com/en-us/entra/identity-platform/v2-oauth2-client-creds-grant-flow).
2. Call Azure REST API using the access token to [list virtual machines in your specified resource group](https://learn.microsoft.com/en-us/rest/api/compute/virtual-machines/list).

### How to run

1. Create a service principal for this sample.

    ```powershell
    $params = @{
        DisplayName = 'CppRest Azure REST API sample [SP]'
        Note        = 'For Azure REST API sample using C++ REST SDK (cpprestsdk).'
        Verbose     = $true
    }
    $sp = New-AzADServicePrincipal @params
    $sp | Format-List @{ Name = 'SP App ID'; Expression = { $_.AppId }}, @{ Name = 'SP Secret'; Expression = { $_.PasswordCredentials.SecretText }}
    ```

2. Assign required roles such as **Reader** to the service principal on your Azure subscription.

    ```powershell
    $params = @{
        Scope         = '/subscriptions/{0}' -f (Get-AzContext).Subscription.Id
        ApplicationId = $sp.AppId
        ObjectType    = 'ServicePrincipal'
        Description   = 'For Azure REST API sample using C++ REST SDK (cpprestsdk).'
    }
    @(
        'Reader'
        # Appropriate roles depend on the Azure REST API that you want to call.
    ) | ForEach-Object -Process { New-AzRoleAssignment @params -RoleDefinitionName $_ }
    ```

3. Run the following command in the **Developer PowerShell** to MSBuild will be able to find vcpkg if you have not run the following command before. See [Tutorial: Install and use packages with MSBuild in Visual Studio](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started-msbuild?pivots=shell-powershell) for more details.

    ```powershell
    vcpkg integrate install
    ```

4. Open the **cpprest-azure-rest-api-sample** solution then set the **oauth2-client-creds-flow** project as startup project.

5. Replace the placeholder text such as `TENANT_ID` to your configuration in the **main.cpp**.

    ```cpp
    // Config for Microsoft Entra ID
    const utility::string_t tenantId = U("TENANT_ID");                     // Tenant ID (GUID) or tenant domain
    const utility::string_t clientId = U("SP_APP_ID");                     // Application (client) ID
    const utility::string_t clientSecret = U("SP_SECRET");                 // Client secret

    // Config for VM info.
    const utility::string_t subscriptionId = U("SUBSCRIPTION_ID");         // Azure subscription ID
    const utility::string_t resourceGroupName = U("RESOURCE_GROUP_NAME");  // Target resource group name
    ```

6. Run the sample from **Debug** - **Start Debugging (F5)**.


## OAuth 2.0 device authorization grant flow sample

This sample do the following:

1. Get an access token from Microsoft Entra ID with your service principal using the [OAuth 2.0 device authorization grant flow](https://learn.microsoft.com/en-us/entra/identity-platform/v2-oauth2-device-code).
2. Call Azure REST API using the access token to [list resource groups](https://learn.microsoft.com/en-us/rest/api/resources/resource-groups/list).

### How to run

1. Create a service principal for this sample.

    ```powershell
    $params = @{
        DisplayName = 'CppRest Azure REST API sample [SP]'
        Note        = 'For Azure REST API sample using C++ REST SDK (cpprestsdk).'
        Verbose     = $true
    }
    $sp = New-AzADServicePrincipal @params
    $sp | Format-List @{ Name = 'SP App ID'; Expression = { $_.AppId }}, @{ Name = 'SP Secret'; Expression = { $_.PasswordCredentials.SecretText }}
    ```

2. Add permission to the service principal for call Azure REST API.

    ```powershell
    $params = @{
        ApplicationId = $sp.AppId
        Type          = 'Scope'
        ApiId         = '797f4846-ba00-4fd7-ba43-dac1f8f63013'  # Microsoft Azure Management
        PermissionId  = '41094075-9dad-400e-a0bd-54e686782033'  # user_impersonation
    }
    Add-AzADAppPermission @params
    ```

3. Allow public client flows on the service principal. See [Desktop app that calls web APIs: App registration](https://learn.microsoft.com/en-us/entra/identity-platform/scenario-desktop-app-registration) and  [Public client and confidential client applications](https://learn.microsoft.com/en-us/entra/identity-platform/msal-client-applications) for details.

    ```powershell
    Update-AzADApplication -ApplicationId $sp.AppId -IsFallbackPublicClient
    ```

4. Assign required roles such as **Reader** to the service principal on your Azure subscription.

    ```powershell
    $params = @{
        Scope         = '/subscriptions/{0}' -f (Get-AzContext).Subscription.Id
        ApplicationId = $sp.AppId
        ObjectType    = 'ServicePrincipal'
        Description   = 'For Azure REST API sample using C++ REST SDK (cpprestsdk).'
    }
    @(
        'Reader'
        # Appropriate roles depend on the Azure REST API that you want to call.
    ) | ForEach-Object -Process { New-AzRoleAssignment @params -RoleDefinitionName $_ }
    ```

5. Run the following command in the **Developer PowerShell for Visual Studio** to MSBuild will be able to find vcpkg if you have not run the following command before. See [Tutorial: Install and use packages with MSBuild in Visual Studio](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started-msbuild?pivots=shell-powershell) for details.

    ```powershell
    vcpkg integrate install
    ```

6. Open the **cpprest-azure-rest-api-sample** solution then set the **oauth2-device-authz-grant-flow** project as startup project.

7. This sample takes three command-line parameters. You need to set these parameters in the project properties if you want to run this sample within Visual Studio. You can set the command-line parameters from **Project** - **Properties** - **Debugging** - **Command Arguments** in Visual Studio.

    ```
    cpprestsample.exe <tenant> <client_id> <subscription_id>
    ```

    Example of **Command Arguments**:

    ```
    aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa bbbbbbbb-bbbb-bbbb-bbbb-bbbbbbbbbbbb cccccccc-cccc-cccc-cccc-cccccccccccc
    ```

8. Run the sample from **Debug** - **Start Debugging (F5)**.


## Notes

- The cpprestsdk is in maintenance mode.
    - [C++ REST SDK](https://github.com/microsoft/cpprestsdk)
        > cpprestsdk is in maintenance mode and we do not recommend its use in new projects. We will continue to fix critical bugs and address security issues.
