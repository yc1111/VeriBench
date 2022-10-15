// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "ccf/app_interface.h"
#include "ccf/common_auth_policies.h"
#include "ccf/http_query.h"
#include "ccf/json_handler.h"
#include "ccf/historical_queries_adapter.h"
#include <vector>
#include <sstream>
#include <fstream>

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
    int n;
    std::string from;
    std::string to;
  };
  DECLARE_JSON_TYPE(YCSB);
  DECLARE_JSON_REQUIRED_FIELDS(YCSB, op, key, val, n, from, to);

  struct Out
  {
    int version;
    std::string msg;
  };
  DECLARE_JSON_TYPE(Out);
  DECLARE_JSON_REQUIRED_FIELDS(Out, version, msg);

  struct SB
  {
    int acc1;
    int acc2;
    int balance;
  };
  DECLARE_JSON_TYPE(SB);
  DECLARE_JSON_REQUIRED_FIELDS(SB, acc1, acc2, balance);
  const std::string saving = "savingStore_";
  const std::string checking = "checkingStore_";

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
          if (!val.has_value()) {
            return ccf::make_error(
              HTTP_STATUS_NOT_FOUND,
              ccf::errors::ResourceNotFound,
              fmt::format("Cannot find value for key \"{}\".", in.key));
          }
          return ccf::make_success(val.value());
        } else if (in.op == 1) {
          auto records_handle = ctx.tx.template rw<Map>(RECORDS);
          records_handle->put(in.key, in.val);
          return ccf::make_success();
        } else if (in.op == 2) {
          return ccf::make_success();
        } else {
          auto records_handle = ctx.tx.template ro<Map>(RECORDS);
          std::vector<std::string> keys, vals;
          records_handle->foreach([this, in, &keys, &vals](const std::string& k, const std::string& v){
            if (k.compare(in.from) < 0) return true;
            else if (k.compare(in.to) > 0) return true;
            else {
              keys.emplace_back(k);
              vals.emplace_back(v);
              return true;
            }
          });
          std::string result;
          for (size_t i = 0; i < keys.size(); ++i) {
            result += keys[i] + ": " + vals[i] + "\n";
          }
          return ccf::make_success(result);
        }
      };

      make_endpoint(
        "/ycsb", HTTP_POST, ccf::json_adapter(ycsb), ccf::no_auth_required)
        .set_auto_schema<YCSB, void>()
        .install();

      auto get_historical = [this](
                              ccf::endpoints::EndpointContext& ctx,
                              ccf::historical::StatePtr historical_state) {
        const auto pack = ccf::jsonhandler::detect_json_pack(ctx.rpc_ctx);

        // Parse id from query
        const auto parsed_query =
          http::parse_query(ctx.rpc_ctx->get_request_query());

        std::string error_reason;
        std::string key;
        if (!http::get_query_value(parsed_query, "key", key, error_reason)) {
          ctx.rpc_ctx->set_error(
            HTTP_STATUS_BAD_REQUEST,
            ccf::errors::InvalidQueryParameterValue,
            std::move(error_reason));
          return;
        }

        auto historical_tx = historical_state->store->create_read_only_tx();
        auto records_handle =
          historical_tx.template ro<Map>(RECORDS);
        const auto v = records_handle->get(key);
        auto version = records_handle->get_version_of_previous_write(key);

        if (v.has_value()) {
          Out out;
          out.msg = v.value();
          out.version = version.value_or(0);
          nlohmann::json j = out;
          ccf::jsonhandler::set_response(std::move(j), ctx.rpc_ctx, pack);
        } else {
          ctx.rpc_ctx->set_response_status(HTTP_STATUS_NO_CONTENT);
        }
      };

      auto is_tx_committed =
        [this](ccf::View view, ccf::SeqNo seqno, std::string& error_reason) {
          return ccf::historical::is_tx_committed_v2(
            consensus, view, seqno, error_reason);
        };

      make_endpoint(
        "/historical",
        HTTP_GET,
        ccf::historical::adapter_v3(
          get_historical, context, is_tx_committed),
        ccf::no_auth_required)
        .set_auto_schema<void, void>()
        .add_query_parameter<std::string>("key")
        .install();

      auto inittpcc = [this](auto& ctx, nlohmann::json&& params) {
        const auto parsed_query =
            http::parse_query(ctx.rpc_ctx->get_request_query());

        std::string error_reason;
        size_t batchid = 0;
        if (!http::get_query_value(parsed_query, "batchid",
            batchid, error_reason)) {
          return ccf::make_error(
            HTTP_STATUS_BAD_REQUEST,
            ccf::errors::InvalidQueryParameterValue,
            std::move(error_reason));
        }

        auto records_handle = ctx.tx.template rw<Map>(RECORDS);
        std::string path = "/external/tpcc";
        std::ifstream infile(path);
        std::string line;
        size_t cnt = 0;
        
        for (size_t i = 0; i < 50000*batchid; ++i) {
          infile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        
        while (std::getline(infile, line)) {
          size_t idx = line.find("\t");
          std::string key = line.substr(0, idx);
          std::string val = line.substr(idx + 1);

          records_handle->put(key, val);
          cnt++;
          if (cnt == 50000) break;
        }
        return ccf::make_success(std::to_string(cnt + 50000*batchid));
      };

      make_endpoint(
        "/inittpcc",
        HTTP_GET,
        ccf::json_adapter(inittpcc),
        ccf::no_auth_required)
        .set_auto_schema<void, void>()
        .add_query_parameter<size_t>("batchid")
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

      auto tpccpayment = [this](auto& ctx, nlohmann::json&& params) {
        const auto in = params.get<TPCC>();
        std::map<std::string, std::string> values;
        auto records_handle = ctx.tx.template rw<Map>(RECORDS);

        auto w_id = in.keys[0];
        auto d_id = in.keys[1];
        auto c_id = in.keys[2];

        auto payment = std::stod(in.keys[3]);
        values.emplace("w_" + w_id,
            records_handle->get("w_" + w_id).value_or(""));
        values.emplace("d_" + d_id + "_" + w_id,
            records_handle->get("d_" + d_id + "_" + w_id).value_or(""));
        values.emplace("c_" + c_id + "_" + d_id + "_" + w_id,
            records_handle->get("c_" + c_id + "_" + d_id + "_" + w_id)
            .value_or(""));

        auto warehouse = parseFields(values["w_" + w_id]);
        auto district = parseFields(values["d_" + d_id + "_" + w_id]);
        auto customer = parseFields(values["c_" + c_id + "_" + d_id + "_" + w_id]);

        auto w_ytd = std::stod(warehouse[7]);
        auto d_ytd = std::stod(district[8]);
        auto c_balance = std::stod(customer[15]);
        auto c_ytd_payment = std::stod(customer[16]);
        auto c_payment_cnt = std::stod(customer[17]);

        warehouse[7] = std::to_string(w_ytd + payment);
        district[8] = std::to_string(d_ytd + payment);
        customer[15] = std::to_string(c_balance - payment);
        customer[16] = std::to_string(c_ytd_payment + payment);
        customer[17] = std::to_string(c_payment_cnt + 1);
        
        records_handle->put("w_" + w_id, fieldsToString(warehouse));
        records_handle->put("d_" + d_id + "_" + w_id,
            fieldsToString(district));
        records_handle->put("c_" + c_id + "_" + d_id + "_" + w_id,
            fieldsToString(customer));
        uint64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        records_handle->put("h_" + c_id + "_" + d_id + "_" + w_id + "_" +
            std::to_string(time), in.keys[3]);
        return ccf::make_success();
      };

      make_endpoint("/tpccpayment", HTTP_POST,
        ccf::json_adapter(tpccpayment), ccf::no_auth_required)
        .set_auto_schema<TPCC, void>()
        .install();

      auto tpccorderstatus = [this](auto& ctx, nlohmann::json&& params) {
        const auto in = params.get<TPCC>();

        std::map<std::string, std::string> values;
        auto records_handle = ctx.tx.template rw<Map>(RECORDS);
      
        auto w_id = in.keys[0];
        auto d_id = in.keys[1];
        auto c_id = in.keys[2];
        values.emplace("w_" + w_id,
            records_handle->get("w_" + w_id).value_or(""));
        values.emplace("d_" + d_id + "_" + w_id,
            records_handle->get("d_" + d_id + "_" + w_id).value_or(""));
        values.emplace("c_" + c_id + "_" + d_id + "_" + w_id,
            records_handle->get("c_" + c_id + "_" + d_id + "_" + w_id)
            .value_or(""));
        values.emplace("c_last_order_" + c_id + "_" + d_id + "_" + w_id,
            records_handle->get("c_last_order_" + c_id + "_" + d_id + "_" +
            w_id).value_or(""));

        auto warehouse = parseFields(values["w_" + w_id]);
        auto district = parseFields(values["d_" + d_id + "_" + w_id]);
        auto customer = parseFields(values["c_" + c_id + "_" + d_id + "_" + w_id]);
        auto o_id = values["c_last_order_" + c_id + "_" + d_id + "_" + w_id];
        if (o_id.compare("") == 0) {
          return ccf::make_error(
            HTTP_STATUS_INTERNAL_SERVER_ERROR,
            ccf::errors::InvalidInput,
            "Invalid order ID");
        }

        values.emplace("o_" + o_id + "_" + d_id + "_" + w_id,
            records_handle->get("o_" + o_id + "_" + d_id + "_" + w_id)
            .value_or(""));

        auto order = parseFields(values["o_" + o_id + "_" + d_id + "_" + w_id]);
        auto ol_cnt = std::stoi(order[6]);
        for (int i = 1; i <= ol_cnt; ++i) {
          values.emplace(
            "ol_" + o_id + "_" + d_id + "_" + w_id + "_" + std::to_string(i),
            records_handle->get("ol_" + o_id + "_" + d_id + "_" + w_id + "_" +
                std::to_string(i)).value_or(""));
        }
        return ccf::make_success();
      };

      make_endpoint("/tpccorderstatus", HTTP_POST,
        ccf::json_adapter(tpccorderstatus), ccf::no_auth_required)
        .set_auto_schema<TPCC, void>()
        .install();


      auto tpccdelivery = [this](auto& ctx, nlohmann::json&& params) {
        const auto in = params.get<TPCC>();
        std::map<std::string, std::string> values;
        auto records_handle = ctx.tx.template rw<Map>(RECORDS);

        auto w_id = in.keys[0];
        auto carrier_id = in.keys[1];

        for (size_t i = 0; i < 10; ++i) {
          values.emplace("d_tax_" + std::to_string(i+1) + "_" + w_id,
              records_handle->get("d_tax_" + std::to_string(i+1) + "_" + w_id)
              .value_or(""));
          values.emplace("next_d_id_" + std::to_string(i+1) + "_" + w_id,
              records_handle->get("next_d_id_" + std::to_string(i+1) +
              "_" + w_id).value_or(""));
        }

        uint64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        std::vector<std::string> o_ids;
        for (size_t i = 0; i < 10; ++i) {
          auto district = parseFields(values["d_tax_" + std::to_string(i+1) +
              "_" + w_id]);
          size_t next_o_id = std::stol(district[1]);
          size_t d_id = 1;
          std::string next_d_id_key = "next_d_id_" + std::to_string(i+1) +
              "_" + w_id;
          if (values.find(next_d_id_key) != values.end() &&
              values[next_d_id_key].size() > 0) {
            d_id = std::stol(values["next_d_id_" + std::to_string(i+1) +
                "_" + w_id]);
          }
          if (d_id >= next_o_id) {
            o_ids.push_back("");
            continue;
          }
          std::string o_id = std::to_string(d_id);
          o_ids.push_back(o_id);
          ++d_id;
          records_handle->put("next_d_id_" + std::to_string(i+1) + "_" + w_id,
              std::to_string(d_id));
          values.emplace(
              "o_" + o_id + "_" + std::to_string(i+1) + "_" + w_id,
              records_handle->get(
              "o_" + o_id + "_" + std::to_string(i+1) + "_" + w_id)
              .value_or(""));
        }
        std::vector<std::string> c_ids;
        std::vector<size_t> ol_cnts;
        for (size_t i = 0; i < 10; ++i) {
          if (o_ids[i].size() == 0) {
            ol_cnts.push_back(0);
            c_ids.push_back("0");
            continue;
          }
          auto order = parseFields(values["o_" + o_ids[i] + "_" +
              std::to_string(i+1) + "_" + w_id]);
          auto ol_cnt = std::stoi(order[6]);
          ol_cnts.push_back(ol_cnt);
          order[5] = carrier_id;
          records_handle->put("o_" + order[0] + "_" + order[1] + "_" +
              order[2], fieldsToString(order));
          for (int j = 1; j <= ol_cnt; ++j) {
            values.emplace("ol_" + o_ids[i] + "_" + std::to_string(i+1) + 
                "_" + w_id + "_" + std::to_string(j),
                records_handle->get("ol_" + o_ids[i] + "_" +
                std::to_string(i+1) + "_" + w_id + "_" + std::to_string(j))
                .value_or(""));
          }
          auto c_id = order[3];
          c_ids.push_back(c_id);
          values.emplace("c_" + c_id + "_" + std::to_string(i+1) + "_" + w_id,
              records_handle->get("c_" + c_id + "_" + std::to_string(i+1) +
              "_" + w_id).value_or(""));
        }

        for (size_t i = 0; i < 10; ++i) {
          if (o_ids[i].size() == 0) continue;
          double total = 0;
          for (size_t j = 1; j <= ol_cnts[i]; ++j) {
            auto orderline = parseFields(values["ol_" + o_ids[i] + "_" +
                std::to_string(i+1) + "_" + w_id + "_" + std::to_string(j)]);
            orderline[6] = std::to_string(time);
            records_handle->put("ol_" + o_ids[i] + "_" + std::to_string(i+1) +
                "_" + w_id + "_" + std::to_string(j),
                fieldsToString(orderline));
            auto ol_amount = std::stod(orderline[8]);
            total += ol_amount;
          }
          auto customer = parseFields(values["c_" + c_ids[i] + "_" +
              std::to_string(i+1) + "_" + w_id]);
          customer[15] = std::to_string(std::stod(customer[15]) + total);
          customer[18] = std::to_string(std::stod(customer[18]) + 1);
          records_handle->put("c_" + c_ids[i] + "_" + std::to_string(i+1) +
              "_" + w_id, fieldsToString(customer));
        }
        return ccf::make_success();
      };

      make_endpoint("/tpccdelivery", HTTP_POST,
        ccf::json_adapter(tpccdelivery), ccf::no_auth_required)
        .set_auto_schema<TPCC, void>()
        .install();


      auto tpccstocklevel = [this](auto& ctx, nlohmann::json&& params) {
        const auto in = params.get<TPCC>();

        std::map<std::string, std::string> values;
        auto records_handle = ctx.tx.template rw<Map>(RECORDS);
      
        auto w_id = in.keys[0];
        auto d_id = in.keys[1];
        auto threshold = std::stol(in.keys[2]);

        values.emplace("d_tax_" + d_id + "_" + w_id,
            records_handle->get("d_tax_" + d_id + "_" + w_id).value_or(""));

        auto district = parseFields(values["d_tax_" + d_id + "_" + w_id]);
        auto o_id = std::stol(district[1]);

        for (auto i = o_id - 20; i < o_id; ++i) {
          values.emplace("o_" + std::to_string(i) + "_" + d_id+ "_" + w_id,
              records_handle->get("o_" + std::to_string(i) + "_" + d_id +
              "_" + w_id).value_or(""));
        }

        std::vector<int> ol_cnts;
        for (auto i = o_id - 20; i < o_id; ++i) {
          std::string order_key = "o_" + std::to_string(i) + "_" + d_id+ "_" + w_id;
          if (values[order_key].size() == 0) {
            return ccf::make_error(
              HTTP_STATUS_INTERNAL_SERVER_ERROR,
              ccf::errors::InvalidInput,
              "Order not found!");
          }
          auto order = parseFields(values[order_key]);
          int ol_cnt = std::stoi(order[6]);
          ol_cnts.push_back(ol_cnt);
          for (int j = 0; j < ol_cnt; ++j) {
            values.emplace("ol_" + std::to_string(i) + "_" + d_id + "_" +
                w_id + "_" + std::to_string(j+1),
                records_handle->get("ol_" + std::to_string(i) + "_" +
                d_id + "_" + w_id + "_" + std::to_string(j+1)).value_or(""));
          }
        }

        size_t ol_cnt_idx = 0;
        std::vector<std::string> i_ids;
        for (auto i = o_id - 20; i < o_id; ++i) {
          for (int j = 0; j < ol_cnts[ol_cnt_idx]; ++j) {
            std::string ol_key = "ol_" + std::to_string(i) + "_" + d_id + "_" +
                w_id + "_" + std::to_string(j+1);
            if (values[ol_key].size() == 0) {
              return ccf::make_error(
                HTTP_STATUS_INTERNAL_SERVER_ERROR,
                ccf::errors::InvalidInput,
                "Order not found!");
            }
            auto orderline = parseFields(values[ol_key]);
            auto i_id = orderline[4];
            i_ids.push_back(i_id);
            auto s_w_id = orderline[5];
            if (s_w_id.compare(w_id) == 0) {
              values.emplace("s_" + i_id + "_" + w_id,
                  records_handle->get("s_" + i_id + "_" + w_id).value_or(""));
            }
          }
          ol_cnt_idx++;
        }

        size_t i_id_idx = 0;
        ol_cnt_idx = 0;
        std::set<std::string> result;
        for (auto i = o_id - 20; i < o_id; ++i) {
          for (auto i_id : i_ids) {
            auto stockstr = values["s_" + i_id + "_" + w_id];
            if (stockstr.size() > 0) {
              auto stock = parseFields(stockstr);
              long quantity = std::stol(stock[2]);
              if (quantity < threshold) {
                result.emplace(stock[0]);
              }
            }
          }
          ol_cnt_idx++;
        }
        return ccf::make_success();
      };

      make_endpoint("/tpccstocklevel", HTTP_POST,
        ccf::json_adapter(tpccstocklevel), ccf::no_auth_required)
        .set_auto_schema<TPCC, void>()
        .install();



      auto sb_amalgamate = [this](auto& ctx, nlohmann::json&& params) {
        const auto in = params.get<SB>();
        std::map<std::string, std::string> values;
        auto records_handle = ctx.tx.template rw<Map>(RECORDS);

        std::string acc1_key = saving + std::to_string(in.acc1);
        std::string acc2_key = checking + std::to_string(in.acc2);
        values.emplace(acc1_key, records_handle->get(acc1_key).value_or(""));
        values.emplace(acc2_key, records_handle->get(acc2_key).value_or(""));
        unsigned int bal1 = stoul(values[acc1_key]);
        unsigned int bal2 = stoul(values[acc2_key]);
        records_handle->put(checking + std::to_string(in.acc1), "0");
        records_handle->put(saving + std::to_string(in.acc2),
            std::to_string(bal1 + bal2));
        return ccf::make_success();
      };

      make_endpoint("/sb_amalgamate", HTTP_POST,
        ccf::json_adapter(sb_amalgamate), ccf::no_auth_required)
        .set_auto_schema<SB, void>()
        .install();

      auto sb_getbalance = [this](auto& ctx, nlohmann::json&& params) {
        const auto in = params.get<SB>();
        std::map<std::string, std::string> values;
        auto records_handle = ctx.tx.template rw<Map>(RECORDS);

        std::string sav_key = saving + std::to_string(in.acc1);
        std::string chk_key = checking + std::to_string(in.acc1);
        values.emplace(sav_key, records_handle->get(sav_key).value_or(""));
        values.emplace(chk_key, records_handle->get(chk_key).value_or(""));
        unsigned int bal1 = stoul(values[sav_key]);
        unsigned int bal2 = stoul(values[chk_key]);
        unsigned int balance = bal1 + bal2;
        return ccf::make_success();
      };

      make_endpoint("/sb_getbalance", HTTP_POST,
        ccf::json_adapter(sb_getbalance), ccf::no_auth_required)
        .set_auto_schema<SB, void>()
        .install();

      auto sb_updatebalance = [this](auto& ctx, nlohmann::json&& params) {
        const auto in = params.get<SB>();
        std::map<std::string, std::string> values;
        auto records_handle = ctx.tx.template rw<Map>(RECORDS);

        std::string chk_key = checking + std::to_string(in.acc1);
        values.emplace(chk_key, records_handle->get(chk_key).value_or(""));
        unsigned int bal1 = stoul(values[chk_key]);
        records_handle->put(chk_key, std::to_string(bal1 + in.balance));
        return ccf::make_success();
      };

      make_endpoint("/sb_updatebalance", HTTP_POST,
        ccf::json_adapter(sb_updatebalance), ccf::no_auth_required)
        .set_auto_schema<SB, void>()
        .install();

      auto sb_updatesav = [this](auto& ctx, nlohmann::json&& params) {
        const auto in = params.get<SB>();
        std::map<std::string, std::string> values;
        auto records_handle = ctx.tx.template rw<Map>(RECORDS);

        std::string sav_key = checking + std::to_string(in.acc1);
        values.emplace(sav_key, records_handle->get(sav_key).value_or(""));
        unsigned int bal1 = stoul(values[sav_key]);
        records_handle->put(sav_key, std::to_string(bal1 + in.balance));
        return ccf::make_success();
      };

      make_endpoint("/sb_updatesav", HTTP_POST,
        ccf::json_adapter(sb_updatesav), ccf::no_auth_required)
        .set_auto_schema<SB, void>()
        .install();

      auto sb_sendpayment = [this](auto& ctx, nlohmann::json&& params) {
        const auto in = params.get<SB>();
        std::map<std::string, std::string> values;
        auto records_handle = ctx.tx.template rw<Map>(RECORDS);

        std::string chk_key1 = checking + std::to_string(in.acc1);
        std::string chk_key2 = checking + std::to_string(in.acc2);
        values.emplace(chk_key1, records_handle->get(chk_key1).value_or(""));
        values.emplace(chk_key2, records_handle->get(chk_key2).value_or(""));
        unsigned int bal1 = stoul(values[chk_key1]);
        unsigned int bal2 = stoul(values[chk_key2]);
        bal1 -= in.balance;
        bal2 += in.balance;
        records_handle->put(chk_key1, std::to_string(bal1));
        records_handle->put(chk_key2, std::to_string(bal2));
        return ccf::make_success();
      };

      make_endpoint("/sb_sendpayment", HTTP_POST,
        ccf::json_adapter(sb_sendpayment), ccf::no_auth_required)
        .set_auto_schema<SB, void>()
        .install();

      auto sb_writecheck = [this](auto& ctx, nlohmann::json&& params) {
        const auto in = params.get<SB>();
        std::map<std::string, std::string> values;
        auto records_handle = ctx.tx.template rw<Map>(RECORDS);

        std::string sav_key = saving + std::to_string(in.acc1);
        std::string chk_key = checking + std::to_string(in.acc1);
        values.emplace(sav_key, records_handle->get(sav_key).value_or(""));
        values.emplace(chk_key, records_handle->get(chk_key).value_or(""));
        int bal1 = stoi(values[chk_key]);
        int bal2 = stoi(values[sav_key]);

        if (in.balance < bal1 + bal2) {
          records_handle->put(chk_key, std::to_string(bal1 - in.balance - 1));
        } else {
          records_handle->put(chk_key, std::to_string(bal1 - in.balance));
        }
        return ccf::make_success();
      };

      make_endpoint("/sb_writecheck", HTTP_POST,
        ccf::json_adapter(sb_writecheck), ccf::no_auth_required)
        .set_auto_schema<SB, void>()
        .install();

      auto initsb = [this](auto& ctx, nlohmann::json&& params) {
        auto records_handle = ctx.tx.template rw<Map>(RECORDS);
        for (int i = 0; i < 100000; ++i) {
          std::string sav_key = saving + std::to_string(i);
          std::string chk_key = checking + std::to_string(i);
          records_handle->put(sav_key, "1000");
          records_handle->put(chk_key, "50");
        }
        return ccf::make_success("init done");
      };

      make_endpoint(
        "/initsb",
        HTTP_GET,
        ccf::json_adapter(initsb),
        ccf::no_auth_required)
        .set_auto_schema<void, void>()
        .install();


      auto initycsb = [this](auto& ctx, nlohmann::json&& params) {
        const auto parsed_query =
            http::parse_query(ctx.rpc_ctx->get_request_query());

        std::string error_reason;
        int version = 0;
        if (!http::get_query_value(parsed_query, "version",
            version, error_reason)) {
          return ccf::make_error(
            HTTP_STATUS_BAD_REQUEST,
            ccf::errors::InvalidQueryParameterValue,
            std::move(error_reason));
        }
        auto records_handle = ctx.tx.template rw<Map>(RECORDS);
        for (int i = 0; i < 100000; ++i) {
          records_handle->put(std::to_string(i),
              std::to_string(i) + "v" + std::to_string(version));
        }
        return ccf::make_success("init done");
      };

      make_endpoint(
        "/initycsb",
        HTTP_GET,
        ccf::json_adapter(initycsb),
        ccf::no_auth_required)
        .set_auto_schema<void, void>()
        .add_query_parameter<int>("version")
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
