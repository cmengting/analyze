/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package config

import (
	"os"
	"time"

	"google.golang.org/protobuf/encoding/protojson"
	"google.golang.org/protobuf/encoding/prototext"
	"naive.systems/analyzer/config/configpb"
)

type Configuration struct {
	configuration *configpb.Configuration
}

func (cfg *Configuration) Configpb() *configpb.Configuration {
	return cfg.configuration
}

func Init(path string) (*Configuration, error) {
	if path == "" {
		path = "/etc/naivesystems/analyzer/worker.config"
	}
	contents, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}
	cfg := &Configuration{
		configuration: &configpb.Configuration{},
	}
	err = prototext.Unmarshal(contents, cfg.configuration)
	if err != nil {
		return nil, err
	}
	return cfg, nil
}

func (cfg *Configuration) LoadRedmineConfigFromJson(path string) error {
	contents, err := os.ReadFile(path)
	if err != nil {
		return err
	}
	cfg.configuration.Redmine = &configpb.RedmineConfiguration{}
	err = protojson.Unmarshal(contents, cfg.configuration.Redmine)
	if err != nil {
		return err
	}
	return nil
}

func (cfg *Configuration) RESTSupportUserAPIKey() string {
	return cfg.configuration.GetSupportUserApiKey()
}

func (cfg *Configuration) RESTAPIKey() string {
	return cfg.configuration.Redmine.GetRestApiKey()
}

func (cfg *Configuration) RESTAPISystemKey() string {
	return cfg.configuration.Redmine.GetRestApiSystemKey()
}

func (cfg *Configuration) RedmineURL() string {
	return cfg.configuration.Redmine.GetRedmineUrl()
}

func (cfg *Configuration) LocalMirrorPath() string {
	return cfg.configuration.GetLocalMirrorPath()
}

func (cfg *Configuration) ImageName() string {
	return cfg.configuration.GetImageName()
}

func (cfg *Configuration) PodmanLoadURLs() []string {
	return cfg.configuration.GetPodmanLoadUrls()
}

func (cfg *Configuration) DocSiteURL() string {
	return cfg.configuration.GetDocSiteUrl()
}

func (cfg *Configuration) GetIssuesPageSize() uint32 {
	return cfg.configuration.GetGetIssuesPageSize()
}

func (cfg *Configuration) ScannerWorkingDir() string {
	return cfg.configuration.GetScannerWorkingDirectory()
}

func (cfg *Configuration) IssPolicyWorkingDir() string {
	return cfg.configuration.GetIssuePolicyWorkingDirectory()
}

func (cfg *Configuration) LogDir() string {
	return cfg.configuration.GetLogDirectory()
}

func (cfg *Configuration) CheckRulesPath() string {
	return cfg.configuration.GetCheckRulesPath()
}

func (cfg *Configuration) CommonFingerprints() []string {
	return cfg.configuration.GetCommonFingerprints()
}

func (cfg *Configuration) StatusIDNew() int32 {
	return cfg.configuration.Redmine.Status.GetNew()
}

func (cfg *Configuration) StatusIDRunning() int32 {
	return cfg.configuration.Redmine.Status.GetRunning()
}

func (cfg *Configuration) StatusIDResolved() int32 {
	return cfg.configuration.Redmine.Status.GetResolved()
}

func (cfg *Configuration) StatusIDReplied() int32 {
	return cfg.configuration.Redmine.Status.GetReplied()
}

func (cfg *Configuration) StatusIDError() int32 {
	return cfg.configuration.Redmine.Status.GetError()
}

func (cfg *Configuration) StatusIDFalsePositive() int32 {
	return cfg.configuration.Redmine.Status.GetFalsePositive()
}

func (cfg *Configuration) StatusIDDuplicate() int32 {
	return cfg.configuration.Redmine.Status.GetDuplicate()
}

func (cfg *Configuration) StatusIDFixed() int32 {
	return cfg.configuration.Redmine.Status.GetFixed()
}

func (cfg *Configuration) StatusIDConfirmed() int32 {
	return cfg.configuration.Redmine.Status.GetConfirmed()
}

func (cfg *Configuration) StatusIDVerifiedFalsePositive() int32 {
	return cfg.configuration.Redmine.Status.GetVerifiedFalsePositive()
}

func (cfg *Configuration) StatusIDIntentional() int32 {
	return cfg.configuration.Redmine.Status.GetIntentional()
}

func (cfg *Configuration) StatusIDVerifiedIntentional() int32 {
	return cfg.configuration.Redmine.Status.GetVerifiedIntentional()
}

func (cfg *Configuration) StatusIDObsolete() int32 {
	return cfg.configuration.Redmine.Status.GetObsolete()
}

func (cfg *Configuration) TrackerIDError() int32 {
	return cfg.configuration.Redmine.Tracker.GetError()
}

func (cfg *Configuration) TrackerIDScan() int32 {
	return cfg.configuration.Redmine.Tracker.GetScan()
}

func (cfg *Configuration) TrackerIDMandatory() int32 {
	return cfg.configuration.Redmine.Tracker.GetMandatory()
}

func (cfg *Configuration) TrackerIDRequired() int32 {
	return cfg.configuration.Redmine.Tracker.GetRequired()
}

func (cfg *Configuration) TrackerIDAdvisory() int32 {
	return cfg.configuration.Redmine.Tracker.GetAdvisory()
}

func (cfg *Configuration) CustomFieldRef() int32 {
	return cfg.configuration.Redmine.CustomField.GetRefId()
}

func (cfg *Configuration) CustomFieldCommitHash() int32 {
	return cfg.configuration.Redmine.CustomField.GetCommitHashId()
}

func (cfg *Configuration) CustomFieldPath() int32 {
	return cfg.configuration.Redmine.CustomField.GetPathId()
}

func (cfg *Configuration) CustomFieldErrorMsg() int32 {
	return cfg.configuration.Redmine.CustomField.GetErrorMessageId()
}

func (cfg *Configuration) CustomFieldLineNumber() int32 {
	return cfg.configuration.Redmine.CustomField.GetLineNumberId()
}

func (cfg *Configuration) CustomFieldLocation() int32 {
	return cfg.configuration.Redmine.CustomField.GetLocationId()
}

func (cfg *Configuration) CustomFieldPolicyChecked() int32 {
	return cfg.configuration.Redmine.CustomField.GetPolicyCheckedId()
}

func (cfg *Configuration) CustomFieldScanTask() int32 {
	return cfg.configuration.Redmine.CustomField.GetScanTaskId()
}

func (cfg *Configuration) CustomFieldLocations() int32 {
	return cfg.configuration.Redmine.CustomField.GetLocationsId()
}

func (cfg *Configuration) CustomFieldDeviationRecId() int32 {
	return cfg.configuration.Redmine.CustomField.GetDeviationRecordsId()
}

func (cfg *Configuration) CustomFieldCompliantRecId() int32 {
	return cfg.configuration.Redmine.CustomField.GetCompliantRecordsId()
}

func (cfg *Configuration) CustomFieldRuleDocLink() int32 {
	return cfg.configuration.Redmine.CustomField.GetRuleDocLink()
}

func (cfg *Configuration) CustomFieldEffectiveLinesId() int32 {
	return cfg.configuration.Redmine.CustomField.GetEffectiveLinesId()
}

func (cfg *Configuration) CustomFieldCheckRulesId() int32 {
	return cfg.configuration.Redmine.CustomField.GetCheckRulesId()
}

func (cfg *Configuration) CustomFieldPorjectCheckRulesId() int32 {
	return cfg.configuration.Redmine.CustomField.GetProjectCheckRulesId()
}

func (cfg *Configuration) CustomFieldArchiveHashId() int32 {
	return cfg.configuration.Redmine.CustomField.GetArchiveHashId()
}

func (cfg *Configuration) CustomFieldAccessTokenID() int32 {
	return cfg.configuration.Redmine.CustomField.GetProjectAccessTokenId()
}

func (cfg *Configuration) CustomFieldSrcDirID() int32 {
	return cfg.configuration.Redmine.CustomField.GetSrcDirId()
}

func (cfg *Configuration) CustomFieldIgnorePatterns() int32 {
	return cfg.configuration.Redmine.CustomField.GetIgnorePatternsId()
}

func (cfg *Configuration) CustomFieldWorkingBranch() int32 {
	return cfg.configuration.Redmine.CustomField.GetWorkingBranchId()
}

func (cfg *Configuration) CustomFieldSeverity() int32 {
	return cfg.configuration.Redmine.CustomField.GetSeverityId()
}

func (cfg *Configuration) CustomFieldProjectType() int32 {
	return cfg.configuration.Redmine.CustomField.GetProjectTypeId()
}

func (cfg *Configuration) CustomFieldQtProPathId() int32 {
	return cfg.configuration.Redmine.CustomField.GetQtProPathId()
}

func (cfg *Configuration) CustomFieldScriptContentsId() int32 {
	return cfg.configuration.Redmine.CustomField.GetScriptContentsId()
}

func (cfg *Configuration) CustomFieldErrorCodeId() int32 {
	return cfg.configuration.Redmine.CustomField.GetErrorCodeId()
}

func (cfg *Configuration) PriorityIDNormal() int32 {
	return cfg.configuration.Redmine.Priority.GetNormal()
}

func (cfg *Configuration) GitBinPath() string {
	return cfg.configuration.GetGitBinPath()
}

func (cfg *Configuration) PodmanBinPath() string {
	return cfg.configuration.GetPodmanBinPath()
}

func (cfg *Configuration) QueryUninitProjs() int32 {
	return cfg.configuration.Redmine.UninitProjsCustomQueryId
}

func (cfg *Configuration) WorkerSleepInterval() time.Duration {
	return time.Second * time.Duration(cfg.configuration.GetWorkerSleepIntervalInSeconds())
}

func (cfg *Configuration) CustomFieldIDGitCloneURL() int {
	return int(cfg.configuration.Redmine.CustomField.GetGitCloneUrlId())
}

func (cfg *Configuration) CustomFieldIDIsInitialized() int {
	return int(cfg.configuration.Redmine.CustomField.GetIsInitializedId())
}

func (cfg *Configuration) CustomFieldIDInitSubmodule() int {
	return int(cfg.configuration.Redmine.CustomField.GetInitSubmoduleId())
}

func (cfg *Configuration) AllowedRefPrefixes() []string {
	return cfg.configuration.GetAllowedRefPrefixes()
}

func (cfg *Configuration) AllowedProjectTypes() []string {
	return cfg.configuration.GetAllowedProjectTypes()
}
