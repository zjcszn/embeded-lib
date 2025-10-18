/*
 *  dynamic_model.c
 *
 *  Copyright 2014-2024 Michael Zillgith
 *
 *  This file is part of libIEC61850.
 *
 *  libIEC61850 is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libIEC61850 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with libIEC61850.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  See COPYING file for the complete license text.
 */

#include "iec61850_server.h"
#include "libiec61850_platform_includes.h"
#include "stack_config.h"

static void
iedModel_emptyVariableInitializer(void)
{
    return;
}

void
IedModel_setIedNameForDynamicModel(IedModel* self, const char* name)
{
    if (self->name)
        GLOBAL_FREEMEM(self->name);

    self->name = StringUtils_copyString(name);
}

IedModel*
IedModel_create(const char* name)
{
    IedModel* self = (IedModel*)GLOBAL_CALLOC(1, sizeof(IedModel));

    if (self)
    {
        if (name)
            self->name = StringUtils_copyString(name);
        else
            self->name = NULL;

        self->rcbs = NULL;

        self->dataSets = NULL;

        self->gseCBs = NULL;

        self->sgcbs = NULL;

        self->lcbs = NULL;

        self->logs = NULL;

        self->initializer = iedModel_emptyVariableInitializer;
    }

    return self;
}

static void
IedModel_addDataSet(IedModel* self, DataSet* dataSet)
{
    if (self->dataSets == NULL)
    {
        self->dataSets = dataSet;
    }
    else
    {
        DataSet* lastDataSet = self->dataSets;

        while (lastDataSet)
        {
            if (lastDataSet->sibling == NULL)
            {
                lastDataSet->sibling = dataSet;
                break;
            }

            lastDataSet = lastDataSet->sibling;
        }
    }
}

static void
IedModel_addLogicalDevice(IedModel* self, LogicalDevice* lDevice)
{
    if (self->firstChild == NULL)
    {
        self->firstChild = lDevice;
    }
    else
    {
        LogicalDevice* sibling = self->firstChild;

        while (sibling->sibling)
            sibling = (LogicalDevice*)sibling->sibling;

        sibling->sibling = (ModelNode*)lDevice;
    }
}

static void
IedModel_addLog(IedModel* self, Log* log)
{
    if (self->logs == NULL)
    {
        self->logs = log;
    }
    else
    {
        Log* lastLog = self->logs;

        while (lastLog->sibling)
            lastLog = lastLog->sibling;

        lastLog->sibling = log;
    }
}

static void
IedModel_addLogControlBlock(IedModel* self, LogControlBlock* lcb)
{
    if (self->lcbs == NULL)
    {
        self->lcbs = lcb;
    }
    else
    {
        LogControlBlock* lastLcb = self->lcbs;

        while (lastLcb->sibling)
            lastLcb = lastLcb->sibling;

        lastLcb->sibling = lcb;
    }
}

static void
IedModel_addReportControlBlock(IedModel* self, ReportControlBlock* rcb)
{
    if (self->rcbs == NULL)
    {
        self->rcbs = rcb;
    }
    else
    {
        ReportControlBlock* lastRcb = self->rcbs;

        while (lastRcb->sibling)
            lastRcb = lastRcb->sibling;

        lastRcb->sibling = rcb;
    }
}

#if (CONFIG_IEC61850_SETTING_GROUPS == 1)
static void
IedModel_addSettingGroupControlBlock(IedModel* self, SettingGroupControlBlock* sgcb)
{
    if (self->sgcbs == NULL)
    {
        self->sgcbs = sgcb;
    }
    else
    {
        SettingGroupControlBlock* lastSgcb = self->sgcbs;

        while (lastSgcb->sibling)
            lastSgcb = lastSgcb->sibling;

        lastSgcb->sibling = sgcb;
    }
}
#endif /* (CONFIG_IEC61850_SETTING_GROUPS == 1) */

static void
IedModel_addGSEControlBlock(IedModel* self, GSEControlBlock* gcb)
{
    if (self->gseCBs == NULL)
    {
        self->gseCBs = gcb;
    }
    else
    {
        GSEControlBlock* lastGcb = self->gseCBs;

        while (lastGcb->sibling)
            lastGcb = lastGcb->sibling;

        lastGcb->sibling = gcb;
    }
}

static void
IedModel_addSMVControlBlock(IedModel* self, SVControlBlock* smvcb)
{
    if (self->svCBs == NULL)
    {
        self->svCBs = smvcb;
    }
    else
    {
        SVControlBlock* lastSvCB = self->svCBs;

        while (lastSvCB->sibling)
            lastSvCB = lastSvCB->sibling;

        lastSvCB->sibling = smvcb;
    }
}

LogicalDevice*
LogicalDevice_createEx(const char* inst, IedModel* parent, const char* ldName)
{
    LogicalDevice* self = (LogicalDevice*)GLOBAL_CALLOC(1, sizeof(LogicalDevice));

    if (self)
    {
        self->name = StringUtils_copyString(inst);
        self->modelType = LogicalDeviceModelType;
        self->parent = (ModelNode*)parent;
        self->sibling = NULL;

        if (ldName)
            self->ldName = StringUtils_copyString(ldName);
        else
            self->ldName = NULL;

        IedModel_addLogicalDevice(parent, self);
    }

    return self;
}

LogicalDevice*
LogicalDevice_create(const char* inst, IedModel* parent)
{
    return LogicalDevice_createEx(inst, parent, NULL);
}

static LogicalNode*
LogicalDevice_getLastLogicalNode(LogicalDevice* self)
{
    LogicalNode* lastNode = (LogicalNode*)self->firstChild;

    LogicalNode* nextNode = lastNode;

    while (nextNode)
    {
        lastNode = nextNode;
        nextNode = (LogicalNode*)nextNode->sibling;
    }

    return lastNode;
}

static void
LogicalDevice_addLogicalNode(LogicalDevice* self, LogicalNode* lNode)
{
    if (self->firstChild == NULL)
    {
        self->firstChild = (ModelNode*)lNode;
    }
    else
    {
        LogicalNode* lastNode = LogicalDevice_getLastLogicalNode(self);

        lastNode->sibling = (ModelNode*)lNode;
    }
}

LogicalNode*
LogicalNode_create(const char* name, LogicalDevice* parent)
{
    LogicalNode* self = (LogicalNode*) GLOBAL_MALLOC(sizeof(LogicalNode));

    if (self)
    {
        self->name = StringUtils_copyString(name);
        self->parent = (ModelNode*) parent;
        self->modelType = LogicalNodeModelType;
        self->firstChild = NULL;
        self->sibling = NULL;

        LogicalDevice_addLogicalNode(parent, self);
    }

    return self;
}

static DataObject*
LogicalNode_getLastDataObject(LogicalNode* self)
{
    DataObject* lastNode = (DataObject*) self->firstChild;

    DataObject* nextNode = lastNode;

    while (nextNode)
    {
        lastNode = nextNode;
        nextNode = (DataObject*) nextNode->sibling;
    }

    return lastNode;

}

static void
LogicalNode_addDataObject(LogicalNode* self, DataObject* dataObject)
{
    if (self->firstChild == NULL)
    {
        self->firstChild = (ModelNode*)dataObject;
    }
    else
    {
        DataObject* lastDataObject = LogicalNode_getLastDataObject(self);

        lastDataObject->sibling = (ModelNode*)dataObject;
    }
}

static void
LogicalNode_addLog(LogicalNode* self, Log* log)
{
    IedModel* model = (IedModel*) self->parent->parent;

    IedModel_addLog(model, log);
}

Log*
Log_create(const char* name, LogicalNode* parent)
{
    Log* self = (Log*) GLOBAL_MALLOC(sizeof(Log));

    if (self)
    {
        self->name = StringUtils_copyString(name);
        self->parent = parent;
        self->sibling = NULL;

        LogicalNode_addLog(parent, self);
    }

    return self;
}

static void
LogicalNode_addLogControlBlock(LogicalNode* self, LogControlBlock* lcb)
{
    IedModel* model = (IedModel*) self->parent->parent;

    IedModel_addLogControlBlock(model, lcb);
}

LogControlBlock*
LogControlBlock_create(const char* name, LogicalNode* parent, const char* dataSetName, const char* logRef, uint8_t trgOps,
        uint32_t intPeriod, bool logEna, bool reasonCode)
{
    LogControlBlock* self = (LogControlBlock*) GLOBAL_MALLOC(sizeof(LogControlBlock));

    if (self)
    {
        self->name = StringUtils_copyString(name);
        self->parent = parent;
        self->sibling = NULL;

        if (dataSetName)
            self->dataSetName = StringUtils_copyString(dataSetName);
        else
            self->dataSetName = NULL;

        if (logRef)
            self->logRef = StringUtils_copyString(logRef);
        else
            self->logRef = NULL;

        self->trgOps = trgOps;
        self->intPeriod = intPeriod;
        self->logEna = logEna;
        self->reasonCode = reasonCode;

        LogicalNode_addLogControlBlock(parent, self);
    }

    return self;
}

const char*
LogControlBlock_getName(LogControlBlock* self)
{
    return self->name;
}

LogicalNode*
LogControlBlock_getParent(LogControlBlock* self)
{
    return self->parent;
}

static void
LogicalNode_addReportControlBlock(LogicalNode* self, ReportControlBlock* rcb)
{
    IedModel* model = (IedModel*) self->parent->parent;

    IedModel_addReportControlBlock(model, rcb);
}

ReportControlBlock*
ReportControlBlock_create(const char* name, LogicalNode* parent, const char* rptId, bool isBuffered, const char*
        dataSetName, uint32_t confRef, uint8_t trgOps, uint8_t options, uint32_t bufTm, uint32_t intgPd)
{
    ReportControlBlock* self = (ReportControlBlock*) GLOBAL_MALLOC(sizeof(ReportControlBlock));

    if (self)
    {
        self->name = StringUtils_copyString(name);
        self->parent = parent;

        if (rptId)
            self->rptId = StringUtils_copyString(rptId);
        else
            self->rptId = NULL;

        self->buffered = isBuffered;

        if (dataSetName)
            self->dataSetName = StringUtils_copyString(dataSetName);
        else
            self->dataSetName = NULL;

        self->confRef = confRef;
        self->trgOps = trgOps;
        self->options = options;
        self->bufferTime = bufTm;
        self->intPeriod = intgPd;
        self->sibling = NULL;
        self->clientReservation[0] = 0; /* no pre-configured client */

        LogicalNode_addReportControlBlock(parent, self);
    }

    return self;
}

void
ReportControlBlock_setPreconfiguredClient(ReportControlBlock* self, uint8_t clientType, const uint8_t* clientAddress)
{
    if (clientType == 4) /* IPv4 address */
    {
        self->clientReservation[0] = 4;
        memcpy(self->clientReservation + 1, clientAddress, 4);
    }
    else if (clientType == 6) /* IPv6 address */
    {
        self->clientReservation[0] = 6;
        memcpy(self->clientReservation + 1, clientAddress, 6);
    }
    else /* no reservation or unknown type */
    {
        self->clientReservation[0] = 0;
    }
}

const char*
ReportControlBlock_getName(ReportControlBlock* self)
{
    return self->name;
}

LogicalNode*
ReportControlBlock_getParent(ReportControlBlock* self)
{
    return self->parent;
}

bool
ReportControlBlock_isBuffered(ReportControlBlock* self)
{
    return self->buffered;
}

#if (CONFIG_IEC61850_SETTING_GROUPS == 1)
static void
LogicalNode_addSettingGroupControlBlock(LogicalNode* self, SettingGroupControlBlock* sgcb)
{
    IedModel* model = (IedModel*) self->parent->parent;

    IedModel_addSettingGroupControlBlock(model, sgcb);
}

SettingGroupControlBlock*
SettingGroupControlBlock_create(LogicalNode* parent, uint8_t actSG, uint8_t numOfSGs)
{
    assert(actSG <= numOfSGs); /* actSG starting with 1 */
    assert(strcmp(parent->name, "LLN0") == 0);

    SettingGroupControlBlock* self = (SettingGroupControlBlock*) GLOBAL_MALLOC(sizeof(SettingGroupControlBlock));

    if (self)
    {
        self->parent = parent;
        self->actSG = actSG;
        self->numOfSGs = numOfSGs;
        self->sibling = NULL;
        self->editSG = 0;

        LogicalNode_addSettingGroupControlBlock(parent, self);
    }

    return self;
}
#endif /* (CONFIG_IEC61850_SETTING_GROUPS == 1) */

static void
LogicalNode_addGSEControlBlock(LogicalNode* self, GSEControlBlock* gcb)
{
    IedModel* model = (IedModel*) self->parent->parent;

    IedModel_addGSEControlBlock(model, gcb);
}

GSEControlBlock*
GSEControlBlock_create(const char* name, LogicalNode* parent, const char* appId, const char* dataSet, uint32_t confRef, bool fixedOffs,
        int minTime, int maxTime)
{
    GSEControlBlock* self = (GSEControlBlock*) GLOBAL_MALLOC(sizeof(GSEControlBlock));

    if (self)
    {
        self->name = StringUtils_copyString(name);
        self->parent = parent;

        if (appId)
            self->appId = StringUtils_copyString(appId);
        else
            self->appId = NULL;

        if (dataSet)
            self->dataSetName = StringUtils_copyString(dataSet);
        else
            self->dataSetName = NULL;

        self->confRev = confRef;
        self->fixedOffs = fixedOffs;
        self->minTime = minTime;
        self->maxTime = maxTime;

        self->address = NULL;

        self->sibling = NULL;

        if (parent)
            LogicalNode_addGSEControlBlock(parent, self);
    }

    return self;
}

static void
LogicalNode_addSMVControlBlock(LogicalNode* self, SVControlBlock* smvcb)
{
    IedModel* model = (IedModel*) self->parent->parent;

    IedModel_addSMVControlBlock(model, smvcb);
}

SVControlBlock*
SVControlBlock_create(const char* name, LogicalNode* parent, const char* svID, const char* dataSet, uint32_t confRev, uint8_t smpMod,
        uint16_t smpRate, uint8_t optFlds, bool isUnicast)
{
    SVControlBlock* self = (SVControlBlock*) GLOBAL_MALLOC(sizeof(SVControlBlock));

    if (self)
    {
        self->name = StringUtils_copyString(name);
        self->parent = parent;

        self->svId = StringUtils_copyString(svID); /* Is there a default value? */

        if (dataSet)
            self->dataSetName = StringUtils_copyString(dataSet);
        else
            self->dataSetName = NULL;

        self->confRev = confRev;

        self->smpMod = smpMod;
        self->smpRate = smpRate;

        self->optFlds = optFlds;
        self->isUnicast = isUnicast;

        self->dstAddress = NULL;
        self->sibling = NULL;

        if (parent)
            LogicalNode_addSMVControlBlock(parent, self);
    }

    return self;
}


const char*
SVControlBlock_getName(SVControlBlock* self)
{
    return self->name;
}

void
SVControlBlock_addPhyComAddress(SVControlBlock* self, PhyComAddress* phyComAddress)
{
    self->dstAddress = phyComAddress;
}

void
GSEControlBlock_addPhyComAddress(GSEControlBlock* self, PhyComAddress* phyComAddress)
{
    self->address = phyComAddress;
}

PhyComAddress*
PhyComAddress_create(uint8_t vlanPriority, uint16_t vlanId, uint16_t appId, uint8_t dstAddress[])
{
    PhyComAddress* self = (PhyComAddress*) GLOBAL_MALLOC(sizeof(PhyComAddress));

    if (self)
    {
        self->vlanPriority = vlanPriority;
        self->vlanId = vlanId;
        self->appId = appId;

        memcpy(self->dstAddress, dstAddress, 6);
    }

    return self;
}

static ModelNode*
DataObject_getLastChild(DataObject* self)
{
    ModelNode* lastNode = self->firstChild;

    ModelNode* nextNode = lastNode;

    while (nextNode)
    {
        lastNode = nextNode;
        nextNode = (ModelNode*)nextNode->sibling;
    }

    return lastNode;
}

static void
DataObject_addChild(DataObject* self, ModelNode* child)
{
    if (self->firstChild == NULL)
    {
        self->firstChild = child;
    }
    else
    {
        ModelNode* lastChild = DataObject_getLastChild(self);

        lastChild->sibling = child;
    }
}

DataObject*
DataObject_create(const char* name, ModelNode* parent, int arrayElements)
{
    DataObject* self = (DataObject*) GLOBAL_MALLOC(sizeof(DataObject));

    if (self)
    {
        self->name = StringUtils_copyString(name);
        self->modelType = DataObjectModelType;
        self->firstChild = NULL;
        self->parent = parent;
        self->sibling = NULL;

        self->elementCount = arrayElements;
        self->arrayIndex = -1;

        if (arrayElements > 0)
        {
            int i;

            for (i = 0; i < arrayElements; i++)
            {
                DataObject* arrayElement = (DataObject*) GLOBAL_MALLOC(sizeof(DataObject));

                if (arrayElement)
                {
                    arrayElement->name = NULL;
                    arrayElement->modelType = DataObjectModelType;
                    arrayElement->firstChild = NULL;
                    arrayElement->parent = (ModelNode*) self;
                    arrayElement->sibling = NULL;

                    arrayElement->elementCount = 0;
                    arrayElement->arrayIndex = i;

                    DataObject_addChild(self, (ModelNode*) arrayElement);
                }
            }
        }

        if (parent->modelType == LogicalNodeModelType)
            LogicalNode_addDataObject((LogicalNode*) parent, self);
        else if (parent->modelType == DataObjectModelType)
            DataObject_addChild((DataObject*) parent, (ModelNode*) self);
    }

    return self;
}

static ModelNode*
DataAttribute_getLastChild(DataAttribute* self)
{
    ModelNode* lastNode = self->firstChild;

    ModelNode* nextNode = lastNode;

    while (nextNode)
    {
        lastNode = nextNode;
        nextNode = (ModelNode*)nextNode->sibling;
    }

    return lastNode;
}

static void
DataAttribute_addChild(DataAttribute* self, ModelNode* child)
{
    if (self->firstChild == NULL)
    {
        self->firstChild = child;
    }
    else
    {
        ModelNode* lastChild = DataAttribute_getLastChild(self);

        lastChild->sibling = child;
    }
}

DataAttribute*
DataAttribute_create(const char* name, ModelNode* parent, DataAttributeType type, FunctionalConstraint fc,
        uint8_t triggerOptions, int arrayElements, uint32_t sAddr)
{
    DataAttribute* self = (DataAttribute*) GLOBAL_MALLOC(sizeof(DataAttribute));

    if (self)
    {
        self->name = StringUtils_copyString(name);
        self->elementCount = arrayElements;
        self->arrayIndex = -1;
        self->modelType = DataAttributeModelType;
        self->type = type;
        self->fc = fc;
        self->firstChild = NULL;
        self->mmsValue = NULL;
        self->parent = parent;
        self->sibling = NULL;
        self->triggerOptions = triggerOptions;
        self->sAddr = sAddr;

        if (arrayElements > 0)
        {
            int i;

            for (i = 0; i < arrayElements; i++)
            {
                DataAttribute* arrayElement = (DataAttribute*) GLOBAL_MALLOC(sizeof(DataAttribute));

                if (arrayElement)
                {
                    arrayElement->name = NULL;
                    arrayElement->elementCount = 0;
                    arrayElement->arrayIndex = i;
                    arrayElement->modelType = DataAttributeModelType;
                    arrayElement->type = type;
                    arrayElement->fc = fc;
                    arrayElement->firstChild = NULL;
                    arrayElement->mmsValue = NULL;
                    arrayElement->parent = (ModelNode*)self;
                    arrayElement->sibling = NULL;
                    arrayElement->triggerOptions = triggerOptions;
                    arrayElement->sAddr = sAddr;

                    DataAttribute_addChild(self, (ModelNode*)arrayElement);
                }
            }
        }

        if (parent->modelType == DataObjectModelType)
            DataObject_addChild((DataObject*) parent, (ModelNode*) self);
        else if (parent->modelType == DataAttributeModelType)
            DataAttribute_addChild((DataAttribute*) parent, (ModelNode*) self);
    }

    return self;
}

DataAttributeType
DataAttribute_getType(DataAttribute* self)
{
    return self->type;
}

FunctionalConstraint
DataAttribute_getFC(DataAttribute* self)
{
    return self->fc;
}

uint8_t
DataAttribute_getTrgOps(DataAttribute* self)
{
    return self->triggerOptions;
}

void
DataAttribute_setValue(DataAttribute* self, MmsValue* value)
{
    if (self->mmsValue)
    {
        MmsValue_update(self->mmsValue, value);
    }
    else
    {
        self->mmsValue = MmsValue_clone(value);
    }
}

DataSet*
DataSet_create(const char* name, LogicalNode* parent)
{
    DataSet* self = (DataSet*) GLOBAL_MALLOC(sizeof(DataSet));

    if (self)
    {
        LogicalDevice* ld = (LogicalDevice*) parent->parent;

        self->name = StringUtils_createString(3, parent->name, "$", name);
        self->elementCount = 0;
        self->sibling = NULL;
        self->logicalDeviceName = ld->name;
        self->fcdas = NULL;

        IedModel_addDataSet((IedModel*) ld->parent, self);
    }

    return self;
}

const char*
DataSet_getName(DataSet* self)
{
    return self->name;
}

int
DataSet_getSize(DataSet* self)
{
    return self->elementCount;
}

DataSetEntry*
DataSet_getFirstEntry(DataSet* self)
{
    return self->fcdas;
}

DataSetEntry*
DataSetEntry_getNext(DataSetEntry* self)
{
    return self->sibling;
}

static void
DataSet_addEntry(DataSet* self, DataSetEntry* newEntry)
{
    self->elementCount++;

    if (self->fcdas == NULL)
        self->fcdas = newEntry;
    else
    {
        DataSetEntry* lastEntry = self->fcdas;

        while (lastEntry)
        {
            if (lastEntry->sibling == NULL) {
                lastEntry->sibling = newEntry;
                break;
            }

            lastEntry = lastEntry->sibling;
        }
    }
}

DataSetEntry*
DataSetEntry_create(DataSet* dataSet, const char* variable, int index, const char* component)
{
    DataSetEntry* self = (DataSetEntry*)GLOBAL_MALLOC(sizeof(DataSetEntry));

    if (self)
    {
        char variableName[130];

        StringUtils_copyStringMax(variableName, 130, variable);

        char* separator = strchr(variableName, '/');

        if (separator)
        {
            *separator = 0;

            self->variableName = StringUtils_copyString(separator + 1);
            self->logicalDeviceName = StringUtils_copyString(variableName);
            self->isLDNameDynamicallyAllocated = true;
        }
        else
        {
            self->variableName = StringUtils_copyString(variable);
            self->logicalDeviceName = dataSet->logicalDeviceName;
            self->isLDNameDynamicallyAllocated = false;
        }

        if (component)
            self->componentName = StringUtils_copyString(component);
        else
            self->componentName = NULL;

        self->index = index;

        self->sibling = NULL;

        self->value = NULL;

        DataSet_addEntry(dataSet, self);
    }

    return self;
}

static void
ModelNode_destroy(ModelNode* modelNode)
{
    if (modelNode)
    {
        if (modelNode->name)
            GLOBAL_FREEMEM(modelNode->name);

        ModelNode* currentChild = modelNode->firstChild;

        while (currentChild)
        {
            ModelNode* nextChild = currentChild->sibling;

            ModelNode_destroy(currentChild);

            currentChild = nextChild;
        }

        if (modelNode->modelType == DataAttributeModelType)
        {
            DataAttribute* dataAttribute = (DataAttribute*)modelNode;

            if (dataAttribute->mmsValue)
            {
                MmsValue_delete(dataAttribute->mmsValue);
                dataAttribute->mmsValue = NULL;
            }
        }

        GLOBAL_FREEMEM(modelNode);
    }
}

void
IedModel_destroy(IedModel* model)
{
    if (model)
    {
        /* delete all model nodes and dynamically created strings */

        /* delete all logical devices */

        LogicalDevice* ld = model->firstChild;

        while (ld)
        {
            if (ld->name)
                GLOBAL_FREEMEM(ld->name);

            if (ld->ldName)
                GLOBAL_FREEMEM(ld->ldName);

            LogicalNode* ln = (LogicalNode*)ld->firstChild;

            while (ln)
            {
                GLOBAL_FREEMEM(ln->name);

                /* delete all data objects */

                DataObject* currentDataObject = (DataObject*)ln->firstChild;

                while (currentDataObject)
                {
                    DataObject* nextDataObject = (DataObject*)currentDataObject->sibling;

                    ModelNode_destroy((ModelNode*)currentDataObject);

                    currentDataObject = nextDataObject;
                }

                LogicalNode* currentLn = ln;
                ln = (LogicalNode*)ln->sibling;

                GLOBAL_FREEMEM(currentLn);
            }

            LogicalDevice* currentLd = ld;
            ld = (LogicalDevice*)ld->sibling;

            GLOBAL_FREEMEM(currentLd);
        }

        /*  delete all data sets */

        DataSet* dataSet = model->dataSets;

        while (dataSet)
        {
            DataSet* nextDataSet = dataSet->sibling;

            GLOBAL_FREEMEM(dataSet->name);

            DataSetEntry* dse = dataSet->fcdas;

            while (dse)
            {
                DataSetEntry* nextDse = dse->sibling;

                if (dse->componentName)
                    GLOBAL_FREEMEM(dse->componentName);

                GLOBAL_FREEMEM(dse->variableName);

                if (dse->isLDNameDynamicallyAllocated)
                    GLOBAL_FREEMEM(dse->logicalDeviceName);

                GLOBAL_FREEMEM(dse);

                dse = nextDse;
            }

            GLOBAL_FREEMEM(dataSet);

            dataSet = nextDataSet;
        }

        /* delete all RCBs */

        ReportControlBlock* rcb = model->rcbs;

        while (rcb)
        {
            ReportControlBlock* nextRcb = rcb->sibling;

            GLOBAL_FREEMEM(rcb->name);

            if (rcb->rptId)
                GLOBAL_FREEMEM(rcb->rptId);

            if (rcb->dataSetName)
                GLOBAL_FREEMEM(rcb->dataSetName);

            GLOBAL_FREEMEM(rcb);

            rcb = nextRcb;
        }

        /* delete all GoCBs */

        GSEControlBlock* gcb = model->gseCBs;

        while (gcb)
        {
            GSEControlBlock* nextGcb = gcb->sibling;

            GLOBAL_FREEMEM(gcb->name);
            GLOBAL_FREEMEM(gcb->appId);
            GLOBAL_FREEMEM(gcb->dataSetName);

            if (gcb->address)
                GLOBAL_FREEMEM(gcb->address);

            GLOBAL_FREEMEM(gcb);

            gcb = nextGcb;
        }

        /* delete setting group controls */

        SettingGroupControlBlock* sgcb = model->sgcbs;

        while (sgcb)
        {
            SettingGroupControlBlock* nextSgcb = sgcb->sibling;

            GLOBAL_FREEMEM(sgcb);

            sgcb = nextSgcb;
        }

        /* delete all LCBs */
        LogControlBlock* lcb = model->lcbs;

        while (lcb)
        {
            LogControlBlock* nextLcb = lcb->sibling;

            if (lcb->name)
                GLOBAL_FREEMEM(lcb->name);

            if (lcb->dataSetName)
                GLOBAL_FREEMEM(lcb->dataSetName);

            if (lcb->logRef)
                GLOBAL_FREEMEM(lcb->logRef);

            GLOBAL_FREEMEM(lcb);

            lcb = nextLcb;
        }

        /* delete all LOGs */
        Log* log = model->logs;

        while (log)
        {
            Log* nextLog = log->sibling;

            if (log->name)
                GLOBAL_FREEMEM(log->name);

            GLOBAL_FREEMEM(log);

            log = nextLog;
        }

        /* delete generic model parts */

        if (model->name)
            GLOBAL_FREEMEM(model->name);

        GLOBAL_FREEMEM(model);
    }
}
