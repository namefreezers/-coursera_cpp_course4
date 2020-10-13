#pragma once


#include <string>

#include "database.h"


class ReadRequest {
public:
    ReadRequest(int id);

    virtual std::string ServeRequestJson(const Database &, bool) const = 0;

protected:
    std::string ServeRequestByJsonData(const std::string &json_data, bool is_last_in_list) const;

private:
    int req_id;
};


class GetStopRequest : public ReadRequest {
public:
    GetStopRequest(int id, std::string stop_name_);

    std::string ServeRequestJson(const Database &db, bool is_last_in_list) const override;

private:
    std::string stop_name;
};


class GetBusRequest : public ReadRequest {
public:
    GetBusRequest(int id, std::string bus_name_);

    std::string ServeRequestJson(const Database &db, bool is_last_in_list) const override;


private:
    std::string bus_name;
};


class GetRouteRequest : public ReadRequest {
public:
    GetRouteRequest(int id, std::string stop_from_, std::string stop_to_);

    std::string ServeRequestJson(const Database &db, bool is_last_in_list) const override;

private:
    std::string stop_from, stop_to;
};
