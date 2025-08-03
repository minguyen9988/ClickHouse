#include "TokenProcessors.h"

#include <Common/logger_useful.h>
#include <Poco/String.h>

namespace DB {

namespace ErrorCodes
{
    extern const int INVALID_CONFIG_PARAMETER;
}

std::unique_ptr<DB::ITokenProcessor> ITokenProcessor::parseTokenProcessor(
        const Poco::Util::AbstractConfiguration & config,
        const String & prefix,
        const String & processor_name)
{
    auto token_cache_lifetime = config.getUInt64(prefix + ".token_cache_lifetime", 3600);
    auto username_claim = config.getString(prefix + ".username_claim", "sub");
    auto groups_claim = config.getString(prefix + ".groups_claim", "groups");

    if (config.hasProperty(prefix + ".provider"))  // One of providers for opaque tokens processing
    {
        auto provider_type = Poco::toLower(config.getString(prefix + ".provider"));

        if (provider_type == "google")
        {
            return std::make_unique<GoogleTokenProcessor>(processor_name, token_cache_lifetime, username_claim, groups_claim);
        }
        else if (provider_type == "azure")
        {
            return std::make_unique<AzureTokenProcessor>(processor_name, token_cache_lifetime, username_claim, groups_claim);
        }
        else if (provider_type == "openid")
        {
            auto verifier_leeway = config.getUInt64(prefix + ".verifier_leeway", 60);
            auto jwks_cache_lifetime = config.getUInt64(prefix + ".jwks_cache_lifetime", 3600);

            if (config.hasProperty(prefix + ".configuration_endpoint"))
            {
                return std::make_unique<OpenIdTokenProcessor>(processor_name, token_cache_lifetime, username_claim,
                                                              groups_claim,
                                                              config.getString(prefix + ".openid_config_endpoint"),
                                                              verifier_leeway,
                                                              jwks_cache_lifetime);
            }
            else
            {
                auto userinfo_endpoint = config.getString(prefix + ".userinfo_endpoint");
                auto token_introspection_endpoint = config.getString(prefix + ".token_introspection_endpoint");
                auto jwks_uri = config.getString(prefix + ".jwks_uri", "");
                return std::make_unique<OpenIdTokenProcessor>(processor_name, token_cache_lifetime, username_claim,
                                                              groups_claim,
                                                              config.getString(prefix + ".userinfo_endpoint"),
                                                              config.getString(prefix + ".token_introspection_endpoint"),
                                                              verifier_leeway,
                                                              config.getString(prefix + ".jwks_uri", ""),
                                                              jwks_cache_lifetime);
            }
        }
    }
    else if (config.hasProperty(prefix + ".algo"))  /// StaticKeyJwtProcessor
    {
        return std::make_unique<StaticKeyJwtProcessor>(processor_name, token_cache_lifetime, username_claim, groups_claim,
                                                       config.getString(prefix + ".claims", ""),
                                                       Poco::toLower(config.getString(prefix + ".algo")),
                                                       config.getString(prefix + ".static_key", ""),
                                                       config.getBool(prefix + ".static_key_in_base64", false),
                                                       config.getString(prefix + ".public_key", ""),
                                                       config.getString(prefix + ".private_key", ""),
                                                       config.getString(prefix + ".public_key_password", ""),
                                                       config.getString(prefix + ".private_key_password", ""));
    }
    else if (config.hasProperty(prefix + ".static_jwks") || config.hasProperty(prefix + ".static_jwks_file"))
    {
        StaticJWKSParams params
        {
                config.getString(prefix + ".static_jwks", ""),
                config.getString(prefix + ".static_jwks_file", "")
        };
        return std::make_unique<JwksJwtProcessor>(processor_name, token_cache_lifetime, username_claim, groups_claim,
                                                  config.getString(prefix + ".claims", ""),
                                                  config.getUInt64(prefix + ".verifier_leeway", 0),
                                                  std::make_shared<StaticJWKS>(params));
    }
    else if (config.hasProperty(prefix + ".jwks_uri"))
    {
        return std::make_unique<JwksJwtProcessor>(processor_name, token_cache_lifetime, username_claim, groups_claim,
                                                  config.getString(prefix + ".claims", ""),
                                                  config.getUInt64(prefix + ".verifier_leeway", 0),
                                                  config.getString(prefix + ".jwks_uri"),
                                                  config.getUInt(prefix + ".jwks_cache_lifetime", 300000));
    }

    throw DB::Exception(ErrorCodes::INVALID_CONFIG_PARAMETER, "Could not parse token processor");
}



}
