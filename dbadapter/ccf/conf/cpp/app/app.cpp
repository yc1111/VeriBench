// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "ccf/app_interface.h"
#include "ccf/common_auth_policies.h"
#include "ccf/http_query.h"
#include "ccf/json_handler.h"
#include <vector>
#include <sstream>

#define FMT_HEADER_ONLY
#include <fmt/format.h>

namespace app
{
  // Key-value store types
  using Map = kv::Map<std::string, std::string>;
  static constexpr auto RECORDS = "records";

  struct YCSB
  {
    int op;
    std::string key;
    std::string val;
  };
  DECLARE_JSON_TYPE(YCSB);
  DECLARE_JSON_REQUIRED_FIELDS(YCSB, op, key, val);

  struct TPCC
  {
    std::vector<std::string> keys;
  };
  DECLARE_JSON_TYPE(TPCC);
  DECLARE_JSON_REQUIRED_FIELDS(TPCC, keys);

  class AppHandlers : public ccf::UserEndpointRegistry
  {
  public:
    AppHandlers(ccfapp::AbstractNodeContext& context) :
      ccf::UserEndpointRegistry(context)
    {
      openapi_info.title = "CCF Sample C++ App";
      openapi_info.description =
        "This minimal CCF C++ application aims to be "
        "used as a template for CCF developers.";
      openapi_info.document_version = "0.0.1";

      auto ycsb = [this](auto& ctx, nlohmann::json&& params) {
        const auto in = params.get<YCSB>();
        if (in.key.empty())
        {
          return ccf::make_error(
            HTTP_STATUS_BAD_REQUEST,
            ccf::errors::InvalidInput,
            "Cannot process empty key");
        }

        if (in.op == 0) {
          auto records_handle = ctx.tx.template ro<Map>(RECORDS);
          auto val = records_handle->get(in.key);
          auto version = records_handle->get_version_of_previous_write(in.key);
          if (!val.has_value()) {
            return ccf::make_error(
              HTTP_STATUS_NOT_FOUND,
              ccf::errors::ResourceNotFound,
              fmt::format("Cannot find value for key \"{}\".", in.key));
          }
          return ccf::make_success(fmt::format("{}, {}",
              version.value_or(0), val.value()));
        } else {
          auto records_handle = ctx.tx.template rw<Map>(RECORDS);
          auto version = records_handle->get_version_of_previous_write(in.key);
          records_handle->put(in.key, in.val);
          return ccf::make_success(fmt::format("{}", version.value_or(0)));
        }
      };

      make_endpoint(
        "/ycsb", HTTP_POST, ccf::json_adapter(ycsb), ccf::no_auth_required)
        .set_auto_schema<YCSB, void>()
        .install();

      auto inittpcc = [this](auto& ctx, nlohmann::json&& params) {
        const auto parsed_query =
          http::parse_query(ctx.rpc_ctx->get_request_query());

        std::string error_reason;
        std::string path;
        if (!http::get_query_value(parsed_query, "path", path, error_reason)) {
          return ccf::make_error(
            HTTP_STATUS_BAD_REQUEST,
            ccf::errors::InvalidQueryParameterValue,
            std::move(error_reason));
        }
        return ccf::make_success(path);
      };

      make_endpoint("/inittpcc", HTTP_POST, ccf::json_adapter(inittpcc),
        ccf::no_auth_required)
        .set_auto_schema<TPCC, void>()
        .add_query_parameter<std::string>("path")
        .install();

      auto tpccneworder = [this](auto& ctx, nlohmann::json&& params) {
        const auto in = params.get<TPCC>();

        std::map<std::string, std::string> values;
        auto records_handle = ctx.tx.template rw<Map>(RECORDS);

        auto w_id = in.keys[0];
        auto d_id = in.keys[1];
        auto c_id = in.keys[2];

        values.emplace("w_tax_" + w_id,
            records_handle->get("w_tax_" + w_id).value_or(""));
        values.emplace("d_tax_" + d_id + "_" + w_id,
            records_handle->get("d_tax_" + d_id + "_" + w_id).value_or(""));
        values.emplace("c_discount_" + c_id + "_" + d_id + "_" + w_id,
            records_handle->get("c_discount_" + c_id + "_" + d_id + "_" + w_id)
            .value_or(""));

        std::vector<std::string> i_ids;
        std::vector<std::string> s_w_ids;
        std::vector<uint32_t> quantities;
        size_t idx = 3;
        while (idx < in.keys.size()) {
          auto i_id = in.keys[idx];
          values.emplace("i_" + i_id,
              records_handle->get("i_" + i_id).value_or(""));
          i_ids.emplace_back(i_id);
          ++idx;

          auto s_w_id = in.keys[idx];
          values.emplace("s_" + i_id + "_" + s_w_id,
              records_handle->get("s_" + i_id + "_" + s_w_id).value_or(""));
          s_w_ids.emplace_back(s_w_id);
          ++idx;

          auto quantity = std::stoi(in.keys[idx]);
          quantities.emplace_back(quantity);
          ++idx;
        }

        uint64_t order_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        auto warehouse = values["w_tax_" + w_id];
        auto district = parseFields(values["d_tax_" + d_id + "_" + w_id]);
        auto customer = values["c_discount_" + c_id + "_" + d_id + "_" + w_id];
        auto w_tax = std::stod(warehouse);
        auto d_tax = std::stod(district[0]);
        auto c_discount = std::stod(customer);
        size_t o_id = std::stol(district[1]);
        int o_all_local = 1;
        for (size_t i = 0; i < i_ids.size(); ++i) {
          auto item = parseFields(values["i_" + i_ids[i]]);
          auto stock = parseFields(values["s_" + i_ids[i] + "_" + s_w_ids[i]]);
          if (stock[1].compare(w_id) != 0) o_all_local = 0;
          auto price = std::stod(item[3]);
          auto dist_info = stock[2 + std::stod(d_id)];
          auto s_quantity = std::stol(stock[2]);
          if (s_quantity > quantities[i]) {
            s_quantity -= quantities[i];
          } else {
            s_quantity = s_quantity - quantities[i] + 100;
          }
          stock[2] = std::to_string(s_quantity);
          records_handle->put("s_" + stock[0] + "_" + stock[1], fieldsToString(stock));
          
          auto ol_amount =
              quantities[i] * price * (1 + w_tax + d_tax) * (1 - c_discount);
          auto ol_key = "ol_" + std::to_string(o_id) + "_" + d_id + "_" + w_id +
                        "_" + std::to_string(i+1);
          std::string ol_val = std::to_string(o_id) + "," + 
                              d_id + "," +
                              w_id + "," +
                              std::to_string(i + 1) + "," +
                              i_ids[i] + "," +
                              s_w_ids[i] + ",," +
                              std::to_string(quantities[i]) + "," +
                              std::to_string(ol_amount) + "," +
                              dist_info;
          records_handle->put(ol_key, ol_val);
        }
        auto order_key = "o_" + std::to_string(o_id) + "_" + d_id + "_" + w_id;
        auto order_val = std::to_string(o_id) + "," + 
                        d_id + "," + 
                        w_id + "," + 
                        c_id + "," + 
                        std::to_string(order_time) + ",," +  // carrier_id empty
                        std::to_string(i_ids.size()) + "," + 
                        std::to_string(o_all_local);
        records_handle->put(order_key, order_val);
        records_handle->put("c_last_order_" + c_id + "_" + d_id + "_" + w_id,
            std::to_string(o_id));
        district[1] = std::to_string(o_id + 1);
        records_handle->put("d_tax_" + d_id + "_" + w_id,
            fieldsToString(district));
        return ccf::make_success();
      };

      make_endpoint("/tpccneworder", HTTP_POST,
        ccf::json_adapter(tpccneworder), ccf::no_auth_required)
        .set_auto_schema<TPCC, void>()
        .install();

    }
  private:
    std::vector<std::string> parseFields(const std::string& row) {
      std::vector<std::string> results;

      std::stringstream ss(row);
      std::string s;
      while (getline(ss, s, ',')) {
        results.emplace_back(s);
      }
      return results;
    }
    std::string fieldsToString(const std::vector<std::string> fields) {
      std::string delimiter = ",";
      std::string result = std::accumulate(std::next(fields.begin()),
          fields.end(), fields[0],
          [&delimiter](std::string& a, const std::string& b) {
              return a + delimiter + b;
          });
      return result;
    }
  };
} // namespace app

namespace ccfapp
{
  std::unique_ptr<ccf::endpoints::EndpointRegistry> make_user_endpoints(
    ccfapp::AbstractNodeContext& context)
  {
    return std::make_unique<app::AppHandlers>(context);
  }
} // namespace ccfapp