#ifndef SABY_UTIL_DATA_EXPORTER_JSON_H_
#define SABY_UTIL_DATA_EXPORTER_JSON_H_

#include "exporter.h"

#include <map>

class JSONDataElement : public DataElementInterface {
public:
    explicit JSONDataElement(UIDType uid) : uid_(uid), sealed_(false) {}
    explicit JSONDataElement()
            : uid_(reinterpret_cast<UIDType>(this)), sealed_(false) {}
    ~JSONDataElement() {}

    void AddData(const std::string &key, long long value) override;
    void AddData(const std::string &key, unsigned long long value) override;
    void AddData(const std::string &key, double value) override;
    void AddData(const std::string &key, bool value) override;
    void AddData(const std::string &key, const std::string &value) override;
    void AddData(const std::string &key, std::nullptr_t) override;
    void AddData(const std::string &key, DataElement &&value) override;
    void AddData(const std::string &key, DataReference &&value) override;
    void AddData(const std::string &key, DataGroup &&value) override;
    void AddData(const std::string &key, DataRefGroup &&value) override;

    void Seal() {
        if (!sealed_) {
            data_str_ += "\n}";
            sealed_ = true;
        }
    }

    UIDType uid() const override { return uid_; }
    const std::string &data_str() const { return data_str_; }

private:
    void InitDataStr();
    void AddKey(const std::string &key);

    UIDType uid_;
    bool sealed_;
    std::string data_str_;
};

class JSONExporter : public ExporterInterface {
public:
    explicit JSONExporter() {}
    ~JSONExporter() {}

    DataElement &NewDataElement(const std::string &name) override {
        elements_[name] = std::make_unique<JSONDataElement>();
        return elements_[name];
    }
    DataGroup &NewDataGroup(const std::string &name) override {
        return groups_[name];
    }
    DataRefGroup &NewDataRefGroup(const std::string &name) override {
        return ref_groups_[name];
    }
    void Export(std::ostream &&os) override;

    void set_version(int major, int minor, int revision) override {
        ver_major_ = major;
        ver_minor_ = minor;
        ver_revision_ = revision;
    }

private:
    int ver_major_, ver_minor_, ver_revision_;
    std::map<std::string, DataElement> elements_;
    std::map<std::string, DataGroup> groups_;
    std::map<std::string, DataRefGroup> ref_groups_;
};

#endif // SABY_UTIL_DATA_EXPORTER_JSON_H_
