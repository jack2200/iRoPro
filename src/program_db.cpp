#include "rapid_pbd/program_db.h"

#include <vector>

#include "mongodb_store/message_store.h"
#include "ros/ros.h"

#include "rapid_pbd_msgs/Program.h"
#include "rapid_pbd_msgs/ProgramInfo.h"
#include "rapid_pbd_msgs/ProgramInfoList.h"

using boost::shared_ptr;
using rapid_pbd_msgs::Program;
using rapid_pbd_msgs::ProgramInfo;
using rapid_pbd_msgs::ProgramInfoList;
using std::pair;
using std::vector;

namespace rapid {
namespace pbd {
ProgramDb::ProgramDb(const ros::NodeHandle& nh,
                     mongodb_store::MessageStoreProxy* db,
                     const ros::Publisher& list_pub)
    : nh_(nh), db_(db), list_pub_(list_pub), program_pubs_() {}

void ProgramDb::Start() { PublishList(); }

void ProgramDb::Insert(const rapid_pbd_msgs::Program& program) {
  std::string id = db_->insert(program);
  PublishList();
}

void ProgramDb::Update(const std::string& db_id,
                       const rapid_pbd_msgs::Program& program) {
  bool success = db_->updateID(db_id, program);
  if (!success) {
    ROS_ERROR("Failed to update program with ID: \"%s\"", db_id.c_str());
    return;
  }
  PublishList();
  PublishProgram(db_id);
}

void ProgramDb::StartPublishingProgramById(const std::string& db_id) {
  if (program_pubs_.find(db_id) != program_pubs_.end()) {
    return;
  }
  vector<shared_ptr<Program> > results;
  bool success = db_->queryID(db_id, results);
  if (!success || results.size() < 1) {
    ROS_ERROR("Can't start publishing program with ID: \"%s\"", db_id.c_str());
    return;
  }
  ros::Publisher pub =
      nh_.advertise<Program>("rapid_pbd/program/" + db_id, 1, true);
  program_pubs_[db_id] = pub;
  program_pubs_[db_id].publish(results[0]);
}

void ProgramDb::Delete(const std::string& db_id) {
  bool success = db_->deleteID(db_id);

  if (success) {
    PublishList();
    if (program_pubs_.find(db_id) != program_pubs_.end()) {
      program_pubs_[db_id].shutdown();
      program_pubs_.erase(db_id);
    }
  } else {
    ROS_ERROR("Could not delete program with ID \"%s\"", db_id.c_str());
  }
}

void ProgramDb::PublishList() {
  vector<pair<shared_ptr<Program>, mongo::BSONObj> > results;
  db_->query<Program>(results);
  ProgramInfoList msg;
  for (size_t i = 0; i < results.size(); ++i) {
    ProgramInfo info;
    info.name = results[i].first->name;
    info.db_id = results[i].second.getField("_id").OID().str();
    msg.programs.push_back(info);
  }
  list_pub_.publish(msg);
}

void ProgramDb::PublishProgram(const std::string& db_id) {
  if (program_pubs_.find(db_id) == program_pubs_.end()) {
    ROS_ERROR("No publisher for program ID: \"%s\"", db_id.c_str());
    return;
  }
  vector<shared_ptr<Program> > results;
  bool success = db_->queryID(db_id, results);
  if (!success || results.size() < 1) {
    ROS_ERROR("Could not republish program with ID: \"%s\"", db_id.c_str());
    return;
  }
  program_pubs_[db_id].publish(results[0]);
}
}  // namespace pbd
}  // namespace rapid