#include "../../include/SmartScanBlue/parser.hpp"

mongodbDAO::mongodbDAO(std::string collname)
{

    collectionName = collname;
}

/*********************************
 * SINGLE DATA INSERTION         *
 * INTO MONGODB SERVER FUCNTIONS.*
 * (INSERTING ONE BY ONE ).      *
*********************************/

/**
void mongodbDAO::insertUnityData(int index, uint64_t timestamp, float wavelength, uint64_t curr_time)
{

    mongocxx::collection coll = db.collection(collectionName);
    long ts = timestamp;
    long ct = curr_time;
    auto builder = bsoncxx::builder::stream::document{};
    bsoncxx::document::value doc_value = builder
                                         //we can delete it (type-peakData)
                                         << "type" << "peakData"
                                         << "curr_time" << (int64_t) ct
                                         << "index" << index
                                         << "timestamp" << (int64_t) ts
                                         << "wavelength" << wavelength
                                         << bsoncxx::builder::stream::finalize;

    bsoncxx::stdx::optional<mongocxx::result::insert_one> result = coll.insert_one(doc_value.view());
}
*/

/***********************************
* MULTIPLE DATA INSERTION          *
* INTO MONGODB SERVER FUCNTIONS.   *
* (INSERTING MULTIPLE (VECTOR)).   *
***********************************/
void mongodbDAO::insertMultipleData(std::vector<cleanPeakData> data, uint64_t curr_time)
{

    mongocxx::collection coll = db.collection(collectionName);
    cleanPeakData data_local;

    std::vector<bsoncxx::document::value> documents;
    while (!data.empty())
    {
        data_local = data.back();
        data.pop_back();

        documents.push_back(
            bsoncxx::builder::stream::document{}
            << "type" << "peakData"
            << "curr_time" << (int64_t)curr_time
            << "index" << data_local.index
            << "timestamp" << (int64_t)data_local.timestamp
            << "wavelength" << data_local.wavelength
            << finalize);

    }
    bsoncxx::stdx::optional<mongocxx::result::insert_many> result =  coll.insert_many(documents);
}

/*********************************
 * SINGLE CONFIG INSERTION       *
 * INTO MONGODB SERVER FUCNTIONS.*
 * (INSERTING ONE BY ONE ).      *
*********************************/

/**
void mongodbDAO::insertConfig(sensorConfig config)
{

    mongocxx::collection coll = db.collection(collectionName);
    int channel = config.channel;
    int grating = config.grating;

    auto builder = bsoncxx::builder::stream::document{};

    bsoncxx::document::value doc_value = builder
                                         //we can delete this also :
                                         << "type" << "config"
                                         << "channel" << channel
                                         << "grating" << grating
                                         << "is_active" << config.is_active
                                         << "wavelength_idle" << config.wavelength_idle
                                         << "wavelength_var" << config.wavelength_var
                                         << "position_x" << config.position.x
                                         << "position_y" << config.position.y
                                         << "position_z" << config.position.z
                                         << bsoncxx::builder::stream::finalize;

    bsoncxx::stdx::optional<mongocxx::result::insert_one> result = coll.insert_one(doc_value.view());

}
*/
/***********************************
* MULTIPLE CONFIG INSERTION        *
* INTO MONGODB SERVER FUCNTIONS.   *
* (INSERTING MULTIPLE (VECTOR)).   *
***********************************/
void mongodbDAO::insertConfigMultiple(std::vector<sensorConfig> config)
{

    mongocxx::collection coll = db.collection(collectionName);
  //  int channel = config.channel;
   // int grating = config.grating;

    std::vector<bsoncxx::document::value> documents;

    for(sensorConfig sc : config){
    documents.push_back(
            bsoncxx::builder::stream::document{}
                                         << "type" << "config"
                                         << "channel" <<(int) sc.channel
                                         << "grating" << (int) sc.grating
                                         << "is_active" << sc.is_active
                                         << "wavelength_idle" << sc.wavelength_idle
                                         << "wavelength_var" << sc.wavelength_var
                                         << "position_x" << sc.position.x
                                         << "position_y" << sc.position.y
                                         << "position_z" << sc.position.z
                                         << finalize);
    }

    bsoncxx::stdx::optional<mongocxx::result::insert_many> result =  coll.insert_many(documents);

}
