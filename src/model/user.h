// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_USER_H_
#define SRC_USER_H_

#include <map>
#include <set>
#include <string>
#include <vector>

#include <json/json.h>  // NOLINT

#include "base_model.h"
#include "batch_update_result.h"
#include "related_data.h"
#include "types.h"
#include "workspace.h"
#include "obm_action.h"

#include <Poco/LocalDateTime.h>
#include <Poco/Types.h>

namespace toggl {

class TOGGL_INTERNAL_EXPORT User : public BaseModel {
    User(ProtectedBase *container)
        : BaseModel(container)
    {}
 public:
    ~User();

    friend class ProtectedBase;

    error EnableOfflineLogin(
        const std::string &password);

    bool HasPremiumWorkspaces() const;
    bool CanAddProjects() const;

    bool CanSeeBillable(locked<Workspace> &ws) const;

    void SetLastTEDate(const std::string &value);

    locked<TimeEntry> RunningTimeEntry();
    locked<const TimeEntry> RunningTimeEntry() const;
    bool IsTracking() const {
        return RunningTimeEntry();
    }

    locked<TimeEntry> Start(
        const std::string &description,
        const std::string &duration,
        const Poco::UInt64 task_id,
        const Poco::UInt64 project_id,
        const std::string project_guid,
        const std::string tags,
        const time_t started,
        const time_t ended,
        const bool stop_current_running);

    locked<TimeEntry> Continue(
        const std::string &GUID,
        const bool manual_mode);

    void Stop();

    // Discard time. Return a new time entry if
    // the discarded time was split into a new time entry
    locked<TimeEntry> DiscardTimeAt(
        const std::string &guid,
        const Poco::Int64 at,
        const bool split_into_new_entry);

    locked<Project> CreateProject(
        const Poco::UInt64 workspace_id,
        const Poco::UInt64 client_id,
        const std::string &client_guid,
        const std::string &client_name,
        const std::string &project_name,
        const bool is_private,
        const std::string &project_color,
        const bool billable);

    locked<Client> CreateClient(
        const Poco::UInt64 workspace_id,
        const std::string &client_name);

    std::string DateDuration(TimeEntry *te) const;

    const std::string &APIToken() const {
        return api_token_;
    }
    void SetAPIToken(const std::string &api_token);

    const Poco::UInt64 &DefaultWID() const {
        return default_wid_;
    }
    void SetDefaultWID(Poco::UInt64 value);

    // Unix timestamp of the user data; returned from API
    const Poco::Int64 &Since() const {
        return since_;
    }
    void SetSince(const Poco::Int64 value);

    bool HasValidSinceDate() const;

    const std::string &Fullname() const {
        return fullname_;
    }
    void SetFullname(const std::string &value);

    const std::string &TimeOfDayFormat() const {
        return timeofday_format_;
    }
    void SetTimeOfDayFormat(const std::string &value);

    const std::string &Email() const {
        return email_;
    }
    void SetEmail(const std::string &value);

    const bool &RecordTimeline() const {
        return record_timeline_;
    }
    void SetRecordTimeline(const bool value);

    const std::string &DurationFormat() const {
        return duration_format_;
    }
    void SetDurationFormat(const std::string &);

    const bool &StoreStartAndStopTime() const {
        return store_start_and_stop_time_;
    }
    void SetStoreStartAndStopTime(const bool value);

    const std::string & OfflineData() const {
        return offline_data_;
    }
    void SetOfflineData(const std::string &);

    const Poco::UInt64& DefaultPID() const {
        return default_pid_;
    }
    void SetDefaultPID(const Poco::UInt64);

    const Poco::UInt64& DefaultTID() const {
        return default_tid_;
    }
    void SetDefaultTID(const Poco::UInt64);

    const bool &CollapseEntries() const {
        return collapse_entries_;
    }
    void SetCollapseEntries(const bool value);

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;

    // Handle related model deletions
    void DeleteRelatedModelsWithWorkspace(const Poco::UInt64 wid);
    void RemoveClientFromRelatedModels(const Poco::UInt64 cid);
    void RemoveProjectFromRelatedModels(const Poco::UInt64 pid);
    void RemoveTaskFromRelatedModels(const Poco::UInt64 tid);

    error LoadUserUpdateFromJSONString(const std::string &json);

    error LoadUserAndRelatedDataFromJSONString(
        const std::string &json,
        const bool &including_related_data);

    error LoadTimeEntriesFromJSONString(const std::string &json);

    error SetAPITokenFromOfflineData(const std::string &password);

    void MarkTimelineBatchAsUploaded(
        const std::vector<TimelineEvent> &events);
    void CompressTimeline();

    std::vector<locked<TimelineEvent>> CompressedTimelineForUI(const Poco::LocalDateTime *date);
    std::vector<locked<TimelineEvent>> CompressedTimelineForUpload(const Poco::LocalDateTime *date = nullptr);

    error UpdateJSON(
        std::vector<TimeEntry *> * const,
        std::string *result) const;

    void LoadObmExperiments(Json::Value const &obm);

    bool LoadUserPreferencesFromJSON(
        Json::Value data);

    bool SetTimeEntryID(
        Poco::UInt64 id,
        locked<TimeEntry> &timeEntry);

    // Get either default or first available WID for items without it
    Poco::UInt64 SupplementaryWID() const;

    static error UserID(
        const std::string &json_data_string,
        Poco::UInt64 *result);

    static error LoginToken(
        const std::string &json_data_string,
        std::string *result);

    static error GenerateOfflineLogin(
        const std::string &email,
        const std::string &password,
        std::string *result);

    bool HasLoadedMore() {
        return has_loaded_more_;
    }

    void ConfirmLoadedMore() {
        has_loaded_more_ = true;
    }

 private:
    void loadUserAndRelatedDataFromJSON(
        Json::Value node,
        const bool &including_related_data);

    void loadUserUpdateFromJSON(
        Json::Value list);

    void loadUserTimeEntryFromJSON(
        Json::Value data,
        std::set<Poco::UInt64> *alive = nullptr);

    std::string dirtyObjectsJSON(std::vector<TimeEntry *> * const) const;

    void processResponseArray(
        std::vector<BatchUpdateResult> * const results,
        std::vector<TimeEntry *> *dirty,
        std::vector<error> *errors);

    error requestJSON(
        const std::string &method,
        const std::string &relative_url,
        const std::string &json,
        const bool authenticate_with_api_token,
        std::string *response_body);

    void parseResponseArray(
        const std::string &response_body,
        std::vector<BatchUpdateResult> *responses);

    std::string generateKey(const std::string &password);

    void loadObmExperimentFromJson(Json::Value const &obm);

    std::vector<locked<TimelineEvent>> CompressedTimeline(
        const Poco::LocalDateTime *date = nullptr, bool is_for_upload = true);

    std::string api_token_ { "" };
    std::string fullname_ { "" };
    std::string email_ { "" };
    std::string timeofday_format_ { "" };
    std::string duration_format_ { "" };
    std::string offline_data_ { "" };
    Poco::UInt64 default_wid_ { 0 };
    // Unix timestamp of the user data; returned from API
    Poco::Int64 since_ { 0 };
    Poco::UInt64 default_pid_ { 0 };
    Poco::UInt64 default_tid_ { 0 };
    bool record_timeline_ { false };
    bool store_start_and_stop_time_ { true };

    bool has_loaded_more_ { false };
    bool collapse_entries_ { false };
};

template<class T>
void deleteZombies(ProtectedContainer<T> &list, const std::set<Poco::UInt64> &alive);

template <typename T>
void deleteRelatedModelsWithWorkspace(const Poco::UInt64 wid, ProtectedContainer<T> &list);

template <typename T>
void removeProjectFromRelatedModels(const Poco::UInt64 pid, ProtectedContainer<T> &list);

}  // namespace toggl

#endif  // SRC_USER_H_
