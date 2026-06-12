#include "../include/action.h"

#include <cstdio>
#include <sqlite3.h>
#include <string>

ActionCommand::ActionCommand() : Action(ActionType::Command) {}

void Action::load_from_db(sqlite3_stmt *stmt) {
	for (int i = 0; i < sqlite3_column_count(stmt); i++) {
		std::string name = sqlite3_column_name(stmt, i);
		if ( name == "id"     ) id     =                            sqlite3_column_int64(stmt, i)  ;
		if ( name == "type"   ) type   = static_cast< ActionType >( sqlite3_column_int  (stmt, i) );
		if ( name == "status" ) status = static_cast< Status     >( sqlite3_column_int  (stmt, i) );
	}
}
void ActionCommand::load_from_db(sqlite3_stmt *stmt) {
  Action::load_from_db(stmt);

	for (int i = 0; i < sqlite3_column_count(stmt); i++) {
		std::string name = sqlite3_column_name(stmt, i);
		if ( name == "command"   ) command   = reinterpret_cast<const char *>(sqlite3_column_text  (stmt, i));
		if ( name == "exit_code" ) exit_code =                                sqlite3_column_int64 (stmt, i) ;
		if ( name == "output"    ) output    = reinterpret_cast<const char *>(sqlite3_column_text  (stmt, i));
	}
}

void Action::save_to_db(sqlite3_stmt *stmt) const {
	int idx_id     = sqlite3_bind_parameter_index( stmt, ":id"     );
	int idx_type   = sqlite3_bind_parameter_index( stmt, ":type"   );
	int idx_status = sqlite3_bind_parameter_index( stmt, ":status" );

	if ( idx_id > 0     ) { sqlite3_bind_int64( stmt, idx_id,      id     ); }
	if ( idx_type > 0   ) { sqlite3_bind_int(   stmt, idx_type,   +type   ); }
	if ( idx_status > 0 ) { sqlite3_bind_int(   stmt, idx_status, +status ); }
}
void ActionCommand::save_to_db(sqlite3_stmt *stmt) const {
  Action::save_to_db(stmt);

  int idx_cmd  = sqlite3_bind_parameter_index( stmt, ":command"   );
  int idx_exit = sqlite3_bind_parameter_index( stmt, ":exit_code" );
  int idx_out  = sqlite3_bind_parameter_index( stmt, ":output"    );

  if ( idx_cmd > 0  ) { sqlite3_bind_text ( stmt, idx_cmd,  command.c_str(), -1, SQLITE_TRANSIENT ); }
  if ( idx_exit > 0 ) { sqlite3_bind_int  ( stmt, idx_exit, exit_code														  ); }
  if ( idx_out > 0  ) { sqlite3_bind_text ( stmt, idx_out,  output.c_str(),  -1, SQLITE_TRANSIENT ); }
}

int operator+(Action::Status s) { return static_cast<int>(s); }
int operator+(ActionType t)     { return static_cast<int>(t); }

void ActionCommand::exec() {
  FILE *pipe = popen((command + " 2>/dev/null").c_str(), "r");

  output.clear();
  char buffer[128];
  while (fgets(buffer, sizeof(buffer), pipe)) { output += buffer; }
  exit_code = WEXITSTATUS(pclose(pipe));
}

void Action::print_debug() {
	printf("check action %lu(%i)\n", id, status); 
}
void ActionCommand::print_debug() {
  printf("check action %lu(%i) : \"%s\" : %i : \"%s\"\n", id, status, command.c_str(), exit_code, output.c_str());
}
