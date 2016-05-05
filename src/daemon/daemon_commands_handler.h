// Copyright (c) 2012-2013 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <boost/lexical_cast.hpp>

#include "console_handler.h"
#include "p2p/net_node.h"
#include "currency_protocol/currency_protocol_handler.h"
#include "rpc/core_rpc_server_commands_defs.h"
#include "common/util.h"
#include "crypto/hash.h"
#include "warnings.h"

PUSH_WARNINGS
DISABLE_VS_WARNINGS(4100)

class daemon_cmmands_handler
{
  nodetool::node_server<currency::t_currency_protocol_handler<currency::core> >& m_srv;
  currency::core_rpc_server &m_rpc_server;
public:
	daemon_cmmands_handler(nodetool::node_server<currency::t_currency_protocol_handler<currency::core> >& srv,  
		currency::core_rpc_server & rpc_server) :m_srv(srv), m_rpc_server(rpc_server)
  {
    m_cmd_binder.set_handler("help", boost::bind(&daemon_cmmands_handler::help, this, _1), "Show this help");
	m_cmd_binder.set_handler("print_pl", boost::bind(&daemon_cmmands_handler::print_pl, this, _1), "Print peer list");
    m_cmd_binder.set_handler("print_cn", boost::bind(&daemon_cmmands_handler::print_cn, this, _1), "Print connections");
    m_cmd_binder.set_handler("print_bc", boost::bind(&daemon_cmmands_handler::print_bc, this, _1), "Print blockchain info in a given blocks range, print_bc <begin_height> [<end_height>]");
    //m_cmd_binder.set_handler("print_bci", boost::bind(&daemon_cmmands_handler::print_bci, this, _1));
    //m_cmd_binder.set_handler("print_bc_outs", boost::bind(&daemon_cmmands_handler::print_bc_outs, this, _1));
    m_cmd_binder.set_handler("print_block", boost::bind(&daemon_cmmands_handler::print_block, this, _1), "Print block, print_block <block_hash> | <block_height>");
    m_cmd_binder.set_handler("print_tx", boost::bind(&daemon_cmmands_handler::print_tx, this, _1), "Print transaction, print_tx <transaction_hash>");
    m_cmd_binder.set_handler("start_mining", boost::bind(&daemon_cmmands_handler::start_mining, this, _1), "Start mining for specified address, start_mining <addr> [threads=1]");
    m_cmd_binder.set_handler("stop_mining", boost::bind(&daemon_cmmands_handler::stop_mining, this, _1), "Stop mining");
    m_cmd_binder.set_handler("print_pool", boost::bind(&daemon_cmmands_handler::print_pool, this, _1), "Print transaction pool (long format)");
    m_cmd_binder.set_handler("print_pool_sh", boost::bind(&daemon_cmmands_handler::print_pool_sh, this, _1), "Print transaction pool (short format)");
	m_cmd_binder.set_handler("print_pool_list", boost::bind(&daemon_cmmands_handler::print_pool_list, this, _1), "Print transaction pool (list format)");
	m_cmd_binder.set_handler("show_hr", boost::bind(&daemon_cmmands_handler::show_hr, this, _1), "Start showing hash rate");
    m_cmd_binder.set_handler("hide_hr", boost::bind(&daemon_cmmands_handler::hide_hr, this, _1), "Stop showing hash rate");
    m_cmd_binder.set_handler("make_alias", boost::bind(&daemon_cmmands_handler::make_alias, this, _1), "Puts alias reservation record into block template, if alias is free");
    m_cmd_binder.set_handler("save", boost::bind(&daemon_cmmands_handler::save, this, _1), "Save blockchain");
    m_cmd_binder.set_handler("tx_stat", boost::bind(&daemon_cmmands_handler::get_transactions_statistics, this, _1), "Calculates transactions statistics");
    m_cmd_binder.set_handler("enable_proxy", boost::bind(&daemon_cmmands_handler::enable_proxy, this, _1), "Enable Socks5 proxy, enable_proxy <proxy_ip_address> <proxy_port>");
    m_cmd_binder.set_handler("disable_proxy", boost::bind(&daemon_cmmands_handler::disable_proxy, this, _1), "Disable Socks5 proxy");
    m_cmd_binder.set_handler("height", boost::bind(&daemon_cmmands_handler::height, this, _1), "Print the blockchain height");
    m_cmd_binder.set_handler("alias", boost::bind(&daemon_cmmands_handler::alias, this, _1), "Print all alias");
	m_cmd_binder.set_handler("getinfo", boost::bind(&daemon_cmmands_handler::getinfo, this, _1), "Print many statistics data");
	m_cmd_binder.set_handler("clear_pool", boost::bind(&daemon_cmmands_handler::clear_pool, this, _1), "Clear pool transactions and keyimages");
	m_cmd_binder.set_handler("clear_peerlist", boost::bind(&daemon_cmmands_handler::clear_peerlist, this, _1), "Clear peers list");
	m_cmd_binder.set_handler("rollback", boost::bind(&daemon_cmmands_handler::rollback, this, _1), "rollback <target_height>, delete all blocks from target_height to current height");
	}

	bool rollback(const std::vector<std::string>& args)
	{
		if (args.empty())
		{
			std::cout << "expected: rollback (<block_height>)" << std::endl;
			return true;
		}

		const std::string& arg = args.front();
		try
		{
			uint64_t height = boost::lexical_cast<uint64_t>(arg);
			if (height == 0 || height > m_srv.get_payload_object().get_core().get_current_blockchain_height())
			{
				std::cout << "wrong height, must be biggger than zero and less than current blockchain height" << ENDL;
				return true;
			}

			bool bSuccess = m_srv.get_payload_object().get_core().get_blockchain_storage().rollback_blockchain(height);
			if (bSuccess = false)
			{
				std::cout << "rollback to " << height << " failed." << ENDL;
			}
			else
			{
				std::cout << "rollback to " << height << " successfully." << ENDL;
			}
		}
		catch (boost::bad_lexical_cast&)
		{
			std::cout << "rollback run error" << std::endl;
		}

		return true;
	}

	bool clear_pool(const std::vector<std::string>& args)
	{
		m_srv.get_payload_object().get_core().get_txpool().clear();
		return true;
	}
	bool clear_peerlist(const std::vector<std::string>& args)
	{
		m_srv.clear();
		return true;
	}
bool getinfo(const std::vector<std::string>& args)
 {
	  currency::COMMAND_RPC_GET_INFO::request req;
	  currency::COMMAND_RPC_GET_INFO::response res = AUTO_VAL_INIT(res);
	  epee::net_utils::connection_context_base cntx = AUTO_VAL_INIT(cntx);
	  if (m_rpc_server.on_get_info(req, res, cntx) == false)
	  {
		  std::cout << "can't get info from rpc server" << ENDL;
		  return false;
	  }

	  std::string json;
	  if (epee::serialization::store_t_to_json(res, json))
	  {
		  std::cout << json << ENDL;
		  return true;
	  }
	  else
	  {
		  std::cout << "error store response to json str" << ENDL;
		  return false;
	  }
  }

  bool start_handling()
  {
    m_cmd_binder.start_handling(&m_srv, "", "");
    return true;
  }

  void stop_handling()
  {
    m_cmd_binder.stop_handling();
  }

private:
  epee::srv_console_handlers_binder<nodetool::node_server<currency::t_currency_protocol_handler<currency::core> > > m_cmd_binder;

  //--------------------------------------------------------------------------------
  std::string get_commands_str()
  {
    std::stringstream ss;
    ss << "Commands: " << ENDL;
    std::string usage = m_cmd_binder.get_usage();
    boost::replace_all(usage, "\n", "\n  ");
    usage.insert(0, "  ");
    ss << usage << ENDL;
    return ss.str();
  }
  //--------------------------------------------------------------------------------
  bool help(const std::vector<std::string>& /*args*/)
  {
    std::cout << get_commands_str() << ENDL;
    return true;
  }
 //--------------------------------------------------------------------------------
  bool height(const std::vector<std::string>& args)
  {
	std::cout << m_srv.get_payload_object().get_core().get_current_blockchain_height() << ENDL;
    return true;
  }
  //--------------------------------------------------------------------------------
  bool alias(const std::vector<std::string>& args)
  {
	std::list<currency::alias_info> aliases;
    m_srv.get_payload_object().get_core().get_blockchain_storage().get_all_aliases(aliases);
	int nCount = 0;
    for(auto a: aliases)
    {
	  std::cout << "index  : " << ++nCount << ENDL;
	  std::cout << "alias  : " << "@" << a.m_alias << ENDL;
	  std::cout << "address: " << currency::get_account_address_as_str(a.m_address) << ENDL;

	  if(a.m_text_comment.size())
		std::cout << "comment: " << a.m_text_comment << ENDL;

	  if(a.m_view_key != currency::null_skey)
		std::cout << "viewkey: " << string_tools::pod_to_hex(a.m_view_key) << ENDL;

	  std::cout << ENDL;
    }
	std::cout << "alias count : " << aliases.size() << ENDL;
    return true;
  }
  //--------------------------------------------------------------------------------
  bool print_pl(const std::vector<std::string>& args)
  {
    m_srv.log_peerlist();
    return true;
  }
  //--------------------------------------------------------------------------------
  bool save(const std::vector<std::string>& args)
  {
    m_srv.get_payload_object().get_core().get_blockchain_storage().store_blockchain();
    return true;
  }
  //--------------------------------------------------------------------------------
  bool get_transactions_statistics(const std::vector<std::string>& args)
  {
    m_srv.get_payload_object().get_core().get_blockchain_storage().print_transactions_statistics();
    return true;
  }
  //--------------------------------------------------------------------------------
  bool show_hr(const std::vector<std::string>& args)
  {
	if(!m_srv.get_payload_object().get_core().get_miner().is_mining()) 
	{
	  std::cout << "Mining is not started. You need start mining before you can see hash rate." << ENDL;
	} else 
	{
	  m_srv.get_payload_object().get_core().get_miner().do_print_hashrate(true);
	}
    return true;
  }
  //--------------------------------------------------------------------------------
  bool hide_hr(const std::vector<std::string>& args)
  {
    m_srv.get_payload_object().get_core().get_miner().do_print_hashrate(false);
    return true;
  }
  //--------------------------------------------------------------------------------
  bool print_bc_outs(const std::vector<std::string>& args)
  {
    if(args.size() != 1)
    {
      std::cout << "need file path as parameter" << ENDL;
      return true;
    }
    m_srv.get_payload_object().get_core().print_blockchain_outs(args[0]);
    return true;
  }
  //--------------------------------------------------------------------------------
  bool print_cn(const std::vector<std::string>& args)
  {
     m_srv.get_payload_object().log_connections();
     return true;
  }
  //--------------------------------------------------------------------------------
  bool print_bc(const std::vector<std::string>& args)
  {
    if(!args.size())
    {
      std::cout << "need block index parameter" << ENDL;
      return false;
    }
    uint64_t start_index = 0;
    uint64_t end_block_parametr = m_srv.get_payload_object().get_core().get_current_blockchain_height();
    if(!string_tools::get_xtype_from_string(start_index, args[0]))
    {
      std::cout << "wrong starter block index parameter" << ENDL;
      return false;
    }
    if(args.size() >1 && !string_tools::get_xtype_from_string(end_block_parametr, args[1]))
    {
      std::cout << "wrong end block index parameter" << ENDL;
      return false;
    }

    m_srv.get_payload_object().get_core().print_blockchain(start_index, end_block_parametr);
    return true;
  }
  //--------------------------------------------------------------------------------
  bool print_bci(const std::vector<std::string>& args)
  {
    m_srv.get_payload_object().get_core().print_blockchain_index();
    return true;
  }
  //--------------------------------------------------------------------------------
  template <typename T>
  static bool print_as_json(T& obj)
  {
    std::cout << currency::obj_to_json_str(obj) << ENDL;
    return true;
  }
  //--------------------------------------------------------------------------------
  bool print_block_by_height(uint64_t height)
  {
    std::list<currency::block> blocks;
    m_srv.get_payload_object().get_core().get_blocks(height, 1, blocks);

    if (1 == blocks.size())
    {
      currency::block& block = blocks.front();
      std::cout << "block_id: " << get_block_hash(block) << ENDL;
      print_as_json(block);
    }
    else
    {
      uint64_t current_height;
      crypto::hash top_id;
      m_srv.get_payload_object().get_core().get_blockchain_top(current_height, top_id);
      std::cout << "block wasn't found. Current block chain height: " << current_height << ", requested: " << height << std::endl;
      return false;
    }

    return true;
  }
  //--------------------------------------------------------------------------------
  bool print_block_by_hash(const std::string& arg)
  {
    crypto::hash block_hash;
    if (!parse_hash256(arg, block_hash))
    {
      return false;
    }

    std::list<crypto::hash> block_ids;
    block_ids.push_back(block_hash);
    std::list<currency::block> blocks;
    std::list<crypto::hash> missed_ids;
    m_srv.get_payload_object().get_core().get_blocks(block_ids, blocks, missed_ids);

    if (1 == blocks.size())
    {
      currency::block block = blocks.front();
      print_as_json(block);
    }
    else
    {
      std::cout << "block wasn't found: " << arg << std::endl;
      return false;
    }

    return true;
  }
  //--------------------------------------------------------------------------------
  bool print_block(const std::vector<std::string>& args)
  {
    if (args.empty())
    {
      std::cout << "expected: print_block (<block_hash> | <block_height>)" << std::endl;
      return true;
    }

    const std::string& arg = args.front();
    try
    {
      uint64_t height = boost::lexical_cast<uint64_t>(arg);
      print_block_by_height(height);
    }
    catch (boost::bad_lexical_cast&)
    {
      print_block_by_hash(arg);
    }

    return true;
  }
  //--------------------------------------------------------------------------------
  bool print_tx(const std::vector<std::string>& args)
  {
    if (args.empty())
    {
      std::cout << "expected: print_tx <transaction hash>" << std::endl;
      return true;
    }

    const std::string& str_hash = args.front();
    crypto::hash tx_hash;
    if (!parse_hash256(str_hash, tx_hash))
    {
      return true;
    }

    std::vector<crypto::hash> tx_ids;
    tx_ids.push_back(tx_hash);
    std::list<currency::transaction> txs;
    std::list<crypto::hash> missed_ids;
    m_srv.get_payload_object().get_core().get_transactions(tx_ids, txs, missed_ids);

    if (1 == txs.size())
    {
      currency::transaction tx = txs.front();
      print_as_json(tx);
    }
    else
    {
      std::cout << "transaction wasn't found: <" << str_hash << '>' << std::endl;
    }

    return true;
  }
  //--------------------------------------------------------------------------------
  bool print_pool(const std::vector<std::string>& args)
  {
	  LOG_PRINT_L0("Pool state: " << ENDL << m_srv.get_payload_object().get_core().print_pool(currency::tx_memory_pool::TX_POOL_FORMAT_LONG));
    return true;
  }
  //--------------------------------------------------------------------------------
  bool print_pool_sh(const std::vector<std::string>& args)
  {
	  LOG_PRINT_L0("Pool state: " << ENDL << m_srv.get_payload_object().get_core().print_pool(currency::tx_memory_pool::TX_POOL_FORMAT_SHORT));
    return true;
  }  
  //--------------------------------------------------------------------------------
  bool print_pool_list(const std::vector<std::string>& args)
  {
	  LOG_PRINT_L0("Pool state: " << ENDL << m_srv.get_payload_object().get_core().print_pool(currency::tx_memory_pool::TX_POOL_FORMAT_LIST));
	  return true;
  }
  //--------------------------------------------------------------------------------
  bool start_mining(const std::vector<std::string>& args)
  {
    if(!args.size())
    {
      std::cout << "Please, specify wallet address to mine for: start_mining <addr> [threads=1]" << std::endl;
      return true;
    }

    currency::account_public_address adr;
    if(!currency::get_account_address_from_str(adr, args.front()))
    {
      std::cout << "target account address has wrong format" << std::endl;
      return true;
    }
    size_t threads_count = 1;
    if(args.size() > 1)
    {
      bool ok = string_tools::get_xtype_from_string(threads_count, args[1]);
      threads_count = (ok && 0 < threads_count) ? threads_count : 1;
    }

    m_srv.get_payload_object().get_core().get_miner().start(adr, threads_count);
    return true;
  }
  //--------------------------------------------------------------------------------
  bool make_alias(const std::vector<std::string>& args)
  {
    if(args.size() != 2)
    {
      std::cout << "Please, specify alias and wallet address to associate with" << std::endl;
      return true;
    }

    if(!currency::validate_alias_name(args[0]))
    {
      std::cout << "Wrong alias name" << std::endl;
      return true;
    }

    currency::alias_info ai = AUTO_VAL_INIT(ai);
    ai.m_alias = args[0];

    if(!currency::get_account_address_from_str(ai.m_address, args[1]))
    {
      std::cout << "target account address has wrong format" << std::endl;
      return true;
    }
    if(m_srv.get_payload_object().get_core().get_miner().set_alias_info(ai))
    {
      std::cout << "Alias \"" << ai.m_alias << "\" set to be committed to blockchain" << std::endl;
    }else 
    {
      std::cout << "Alias \"" << ai.m_alias << "\" failed to be committed to blockchain" << std::endl;
    }
    return true;
  }  
  //--------------------------------------------------------------------------------
  bool stop_mining(const std::vector<std::string>& args)
  {
    m_srv.get_payload_object().get_core().get_miner().stop();
    return true;
  }
  //--------------------------------------------------------------------------------
  bool enable_proxy(const std::vector<std::string>& args)
  {
    if(args.size() != 2)
    {
      std::cout << "Please, specify the ip address and the port of Socks5 proxy "<< std::endl;
      return true;
    }
	std::string proxy_ip;
	int proxy_port = 0;
	bool bEnable_proxy = m_srv.get_proxy_status(proxy_ip,proxy_port);

	if(bEnable_proxy) 
    {
      std::cout << "Proxy:  "<< proxy_ip << ":" << proxy_port << " already enabled. Don't enable it again. " << std::endl;
      return true;
    }

	proxy_port = atoi(args[1].c_str());
	if (m_srv.enable_proxy(true, true, args[0], proxy_port))
	{
		std::cout << "Proxy:  " << args[0] << ":" << proxy_port << " enabled now. " << std::endl;
	}
	
    return true;
  }
 //--------------------------------------------------------------------------------
  bool disable_proxy(const std::vector<std::string>& args)
  {
 	std::string proxy_ip;
	int proxy_port = 0;
	bool bEnable_proxy = m_srv.get_proxy_status(proxy_ip,proxy_port);
	
	if(bEnable_proxy == false) 
    {
      std::cout << "Proxy:  "<< proxy_ip << ":" << proxy_port << " already disabled. Don't disable it again. " << std::endl;
      return true;
    }
	if(m_srv.enable_proxy(false,true))
	{
		std::cout << "Proxy:  "<< proxy_ip << ":" << proxy_port << " disabled now. " << std::endl;
	 }
		
     return true;
  }
 //--------------------------------------------------------------------------------
};
POP_WARNINGS
