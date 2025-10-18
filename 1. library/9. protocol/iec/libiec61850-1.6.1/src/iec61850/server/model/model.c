/*
 *  model.c
 *
 *  Copyright 2013-2024 Michael Zillgith
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

#include "iec61850_model.h"

#include "libiec61850_platform_includes.h"
#include "stack_config.h"

static void
setAttributeValuesToNull(ModelNode* node)
{
    if (node->modelType == DataAttributeModelType)
    {
        DataAttribute* da = (DataAttribute*)node;

        da->mmsValue = NULL;
    }

    ModelNode* child = node->firstChild;

    while (child)
    {
        setAttributeValuesToNull(child);
        child = child->sibling;
    }
}

void
IedModel_setIedName(IedModel* self, const char* name)
{
    self->name = (char*)name;
}

void
IedModel_setAttributeValuesToNull(IedModel* iedModel)
{
    LogicalDevice* ld = iedModel->firstChild;

    while (ld)
    {
        LogicalNode* ln = (LogicalNode*)ld->firstChild;

        while (ln)
        {
            ModelNode* node = ln->firstChild;

            while (node)
            {
                setAttributeValuesToNull(node);
                node = node->sibling;
            }

            ln = (LogicalNode*)ln->sibling;
        }

        ld = (LogicalDevice*)ld->sibling;
    }
}

int
IedModel_getLogicalDeviceCount(IedModel* self)
{
    if (self->firstChild == NULL)
        return 0;

    LogicalDevice* logicalDevice = self->firstChild;

    int ldCount = 1;

    while (logicalDevice->sibling)
    {
        logicalDevice = (LogicalDevice*)logicalDevice->sibling;
        ldCount++;
    }

    return ldCount;
}

DataSet*
IedModel_lookupDataSet(IedModel* self, const char* dataSetReference /* e.g. ied1Inverter/LLN0$dataset1 */)
{
    DataSet* dataSet = self->dataSets;

    const char* separator = strchr(dataSetReference, '/');

    if (separator == NULL)
        return NULL;

    int ldNameLen = separator - dataSetReference;

    char domainName[65];

    int modelNameLen = strlen(self->name);

    if (modelNameLen > 64)
        return NULL;

    memcpy(domainName, self->name, modelNameLen);

    while (dataSet)
    {
        LogicalDevice* ld = IedModel_getDeviceByInst(self, dataSet->logicalDeviceName);

        if (ld)
        {
            if (ld->ldName == NULL)
            {
                /* LD name = IEDName + ldInst */
                domainName[modelNameLen] = 0;
                StringUtils_appendString(domainName, 65, dataSet->logicalDeviceName);

                if (strncmp(domainName, dataSetReference, ldNameLen) == 0)
                {
                    if (strcmp(dataSet->name, separator + 1) == 0)
                    {
                        return dataSet;
                    }
                }
            }
            else
            {
                /* functional naming */
                if (strncmp(ld->ldName, dataSetReference, ldNameLen) == 0)
                {
                    if (strcmp(dataSet->name, separator + 1) == 0)
                    {
                        return dataSet;
                    }
                }
            }
        }

        dataSet = dataSet->sibling;
    }

    return NULL;
}

LogicalDevice*
IedModel_getDevice(IedModel* self, const char* deviceName)
{
    LogicalDevice* device = self->firstChild;

    while (device)
    {
        if (device->ldName)
        {
            /* functional naming */
            if (strcmp(device->ldName, deviceName) == 0)
                return device;
        }
        else
        {
            /* LD name = IEDName + ldInst */
            char domainName[65];

            StringUtils_concatString(domainName, 65, self->name, device->name);

            if (strcmp(domainName, deviceName) == 0)
                return device;
        }

        device = (LogicalDevice*)device->sibling;
    }

    return NULL;
}

LogicalDevice*
IedModel_getDeviceByInst(IedModel* self, const char* ldInst)
{
    LogicalDevice* device = self->firstChild;

    while (device)
    {
        if (strcmp(device->name, ldInst) == 0)
            return device;

        device = (LogicalDevice*)device->sibling;
    }

    return NULL;
}

LogicalDevice*
IedModel_getDeviceByIndex(IedModel* self, int index)
{
    LogicalDevice* logicalDevice = self->firstChild;

    int currentIndex = 0;

    while (logicalDevice)
    {
        if (currentIndex == index)
            return logicalDevice;

        currentIndex++;

        logicalDevice = (LogicalDevice*)logicalDevice->sibling;
    }

    return NULL;
}

static DataAttribute*
ModelNode_getDataAttributeByMmsValue(ModelNode* self, MmsValue* value)
{
    ModelNode* node = self->firstChild;

    while (node)
    {
        if (node->modelType == DataAttributeModelType)
        {
            DataAttribute* da = (DataAttribute*)node;

            if (da->mmsValue == value)
                return da;
        }

        DataAttribute* da = ModelNode_getDataAttributeByMmsValue(node, value);

        if (da)
            return da;

        node = node->sibling;
    }

    return NULL;
}

DataAttribute*
IedModel_lookupDataAttributeByMmsValue(IedModel* model, MmsValue* value)
{
    LogicalDevice* ld = model->firstChild;

    while (ld)
    {
        DataAttribute* da = ModelNode_getDataAttributeByMmsValue((ModelNode*)ld, value);

        if (da)
            return da;

        ld = (LogicalDevice*)ld->sibling;
    }

    return NULL;
}

static ModelNode*
getChildWithShortAddress(ModelNode* node, uint32_t sAddr)
{
    ModelNode* child;

    child = node->firstChild;

    while (child)
    {
        if (child->modelType == DataAttributeModelType)
        {
            DataAttribute* da = (DataAttribute*)child;

            if (da->sAddr == sAddr)
                return child;
        }

        ModelNode* childChild = getChildWithShortAddress(child, sAddr);

        if (childChild)
            return childChild;

        child = child->sibling;
    }

    return NULL;
}

ModelNode*
IedModel_getModelNodeByShortAddress(IedModel* model, uint32_t sAddr)
{
    ModelNode* node = NULL;

    LogicalDevice* ld = (LogicalDevice*)model->firstChild;

    while (ld)
    {
        LogicalNode* ln = (LogicalNode*)ld->firstChild;

        while (ln)
        {
            ModelNode* doNode = ln->firstChild;

            while (doNode)
            {
                ModelNode* matchingNode = getChildWithShortAddress(doNode, sAddr);

                if (matchingNode)
                    return matchingNode;

                doNode = doNode->sibling;
            }

            ln = (LogicalNode*)ln->sibling;
        }

        ld = (LogicalDevice*)ld->sibling;
    }

    return node;
}

ModelNode*
IedModel_getModelNodeByObjectReference(IedModel* model, const char* objectReference)
{
    assert(strlen(objectReference) < 129);

    char objRef[130];

    StringUtils_copyStringMax(objRef, 130, objectReference);

    char* separator = strchr(objRef, '/');

    if (separator)
        *separator = 0;

    LogicalDevice* ld = IedModel_getDevice(model, objRef);

    if (ld == NULL)
        return NULL;

    if ((separator == NULL) || (*(separator + 1) == 0))
        return (ModelNode*)ld;

    return ModelNode_getChild((ModelNode*)ld, separator + 1);
}

#if (CONFIG_IEC61850_SAMPLED_VALUES_SUPPORT == 1)

SVControlBlock*
IedModel_getSVControlBlock(IedModel* self, LogicalNode* parentLN, const char* svcbName)
{
    SVControlBlock* retVal = NULL;

    SVControlBlock* svCb = self->svCBs;

    while (svCb)
    {
        if ((svCb->parent == parentLN) && (strcmp(svCb->name, svcbName) == 0))
        {
            retVal = svCb;
            break;
        }

        svCb = svCb->sibling;
    }

    return retVal;
}

#endif /* (CONFIG_IEC61850_SAMPLED_VALUES_SUPPORT == 1) */

ModelNode*
IedModel_getModelNodeByShortObjectReference(IedModel* model, const char* objectReference)
{
    assert((strlen(model->name) + strlen(objectReference)) < 130);

    char objRef[130];

    StringUtils_copyStringMax(objRef, 130, objectReference);

    char* separator = strchr(objRef, '/');

    if (separator)
        *separator = 0;

    char ldInst[65];
    StringUtils_copyStringMax(ldInst, 65, objRef);

    LogicalDevice* ld = IedModel_getDeviceByInst(model, ldInst);

    if (ld == NULL)
    {
        if (DEBUG_IED_SERVER)
        {
            printf("IED_SERVER: LD (%s) not found\n", ldInst);
        }

        return NULL;
    }

    if ((separator == NULL) || (*(separator + 1) == 0))
        return (ModelNode*)ld;

    return ModelNode_getChild((ModelNode*)ld, separator + 1);
}

bool
DataObject_hasFCData(DataObject* dataObject, FunctionalConstraint fc)
{
    ModelNode* modelNode = dataObject->firstChild;

    while (modelNode)
    {
        if (modelNode->modelType == DataAttributeModelType)
        {
            DataAttribute* dataAttribute = (DataAttribute*)modelNode;

            if (dataAttribute->fc == fc)
                return true;
        }
        else if (modelNode->modelType == DataObjectModelType)
        {
            if (DataObject_hasFCData((DataObject*)modelNode, fc))
                return true;
        }

        modelNode = modelNode->sibling;
    }

    return false;
}

bool
LogicalNode_hasFCData(LogicalNode* node, FunctionalConstraint fc)
{
    DataObject* dataObject = (DataObject*)node->firstChild;

    while (dataObject)
    {
        if (DataObject_hasFCData(dataObject, fc))
            return true;

        dataObject = (DataObject*)dataObject->sibling;
    }

    return false;
}

DataSet*
LogicalNode_getDataSet(LogicalNode* self, const char* dataSetName)
{
    assert(self->modelType == LogicalNodeModelType);
    assert(dataSetName != NULL);

    char dsName[66];

    LogicalDevice* ld = (LogicalDevice*)self->parent;

    if (strlen(dataSetName) > 32)
    {
        if (DEBUG_IED_SERVER)
        {
            printf("IED_SERVER: LogicalNode_getDataSet - data set name %s too long!\n", dataSetName);
        }

        goto exit_error;
    }

    StringUtils_createStringInBuffer(dsName, 66, 3, self->name, "$", dataSetName);

    IedModel* iedModel = (IedModel*)ld->parent;

    DataSet* ds = iedModel->dataSets;

    while (ds)
    {
        if (strcmp(ds->logicalDeviceName, ld->name) == 0)
        {
            if (strcmp(ds->name, dsName) == 0)
            {
                return ds;
            }
        }

        ds = ds->sibling;
    }

exit_error:
    return NULL;
}

int
LogicalDevice_getLogicalNodeCount(LogicalDevice* logicalDevice)
{
    int lnCount = 0;

    LogicalNode* logicalNode = (LogicalNode*)logicalDevice->firstChild;

    while (logicalNode)
    {
        logicalNode = (LogicalNode*)logicalNode->sibling;
        lnCount++;
    }

    return lnCount;
}

ModelNode*
LogicalDevice_getChildByMmsVariableName(LogicalDevice* logicalDevice, const char* mmsVariableName)
{
    const char* separator = strchr(mmsVariableName, '$');

    if (separator == NULL)
        return NULL;

    if (strlen(separator) > 4)
    {
        char fcString[3];
        char nameRef[65];

        fcString[0] = separator[1];
        fcString[1] = separator[2];
        fcString[2] = 0;

        const char* strpos = mmsVariableName;

        int targetPos = 0;

        while (strpos < separator)
        {
            nameRef[targetPos++] = strpos[0];
            strpos++;
        }

        nameRef[targetPos++] = '.';

        strpos = separator + 4;

        while (strpos[0] != 0)
        {
            nameRef[targetPos++] = strpos[0];
            strpos++;
        }

        nameRef[targetPos++] = 0;

        StringUtils_replace(nameRef, '$', '.');

        FunctionalConstraint fc = FunctionalConstraint_fromString(fcString);

        return ModelNode_getChildWithFc((ModelNode*)logicalDevice, nameRef, fc);
    }

    return NULL;
}

static int
createObjectReference(ModelNode* node, char* objectReference, int bufSize, bool withoutIedName)
{
    int bufPos;
    int arrayIndex = -1;

    if (node->modelType != LogicalNodeModelType)
    {
        bufPos = createObjectReference(node->parent, objectReference, bufSize, withoutIedName);

        if (node->modelType == DataAttributeModelType)
        {
            arrayIndex = ((DataAttribute*)(node))->arrayIndex;
        }
        else if (node->modelType == DataObjectModelType)
        {
            arrayIndex = ((DataObject*)(node))->arrayIndex;
        }

        if (bufPos == -1)
            return -1;

        if (arrayIndex < 0)
        {
            if (bufPos < bufSize)
                objectReference[bufPos++] = '.';
            else
                return -1;
        }
    }
    else
    {
        LogicalNode* lNode = (LogicalNode*)node;

        LogicalDevice* lDevice = (LogicalDevice*)lNode->parent;

        IedModel* iedModel = (IedModel*)lDevice->parent;

        bufPos = 0;

        if (withoutIedName)
        {
            objectReference[0] = 0;
            StringUtils_appendString(objectReference, bufSize, lDevice->name);
        }
        else
        {
            if (lDevice->ldName)
            {
                StringUtils_copyStringMax(objectReference, bufSize, lDevice->ldName);
            }
            else
            {
                StringUtils_concatString(objectReference, bufSize, iedModel->name, lDevice->name);
            }
        }

        bufPos += strlen(objectReference);

        if (bufPos < (bufSize - 2))
            objectReference[bufPos++] = '/';
        else
            return -1;
    }

    if (node->name)
    {
        /* append own name */
        int nameLength = strlen(node->name);

        if (bufPos + nameLength < bufSize)
        {
            int i;
            for (i = 0; i < nameLength; i++)
            {
                objectReference[bufPos++] = node->name[i];
            }

            return bufPos;
        }
        else
        {
            return -1;
        }
    }

    if (arrayIndex > -1)
    {
        char arrayIndexStr[11];

        snprintf(arrayIndexStr, 11, "%d", arrayIndex);

        int arrayIndexStrLength = strlen(arrayIndexStr);

        if (bufPos + arrayIndexStrLength + 2 < bufSize)
        {
            int i;

            objectReference[bufPos++] = '(';

            for (i = 0; i < arrayIndexStrLength; i++)
            {
                objectReference[bufPos++] = arrayIndexStr[i];
            }
            objectReference[bufPos++] = ')';
        }
        else
            return -1;
    }

    return bufPos;
}

char*
ModelNode_getObjectReference(ModelNode* node, char* objectReference)
{
    return ModelNode_getObjectReferenceEx(node, objectReference, false);
}

char*
ModelNode_getObjectReferenceEx(ModelNode* node, char* objectReference, bool withoutIedName)
{
    bool allocated = false;

    if (objectReference == NULL)
    {
        objectReference = (char*)GLOBAL_MALLOC(130);
        allocated = true;
    }

    if (objectReference)
    {
        int bufPos = createObjectReference(node, objectReference, 130, withoutIedName);

        if (bufPos == -1)
        {
            if (allocated)
                GLOBAL_FREEMEM(objectReference);

            return NULL;
        }

        if (bufPos < 130)
            objectReference[bufPos] = 0;
        else
            objectReference[129] = 0;
    }

    return objectReference;
}

int
ModelNode_getChildCount(ModelNode* modelNode)
{
    int childCount = 0;

    ModelNode* child = modelNode->firstChild;

    while (child)
    {
        childCount++;
        child = child->sibling;
    }

    return childCount;
}

ModelNode*
ModelNode_getChild(ModelNode* self, const char* name)
{
    /* check for element separator */
    const char* separator = strchr(name, '.');

    /* allow first character to be "." */
    if (separator == name)
        name++;

    /* check for array separator */
    const char* arraySeparator = strchr(name, '(');

    if (arraySeparator)
    {
        const char* arraySeparator2 = strchr(arraySeparator, ')');

        if (arraySeparator2)
        {
            int idx = (int)strtol(arraySeparator + 1, NULL, 10);

            ModelNode* arrayNode = NULL;

            if (name == arraySeparator)
            {
                arrayNode = ModelNode_getChildWithIdx(self, idx);
            }
            else
            {
                char nameCopy[65];

                const char* pos = name;

                int cpyIdx = 0;

                while (pos < arraySeparator)
                {
                    nameCopy[cpyIdx] = *pos;
                    cpyIdx++;
                    pos++;
                }

                nameCopy[cpyIdx] = 0;

                ModelNode* childNode = ModelNode_getChild(self, nameCopy);

                if (childNode)
                {
                    arrayNode = ModelNode_getChildWithIdx(childNode, idx);
                }
                else
                    return NULL;
            }

            if (arrayNode)
            {
                if (*(arraySeparator2 + 1) == 0)
                {
                    return arrayNode;
                }
                else
                {
                    if (*(arraySeparator2 + 1) == '.')
                        return ModelNode_getChild(arrayNode, arraySeparator2 + 2);
                    else
                        return ModelNode_getChild(arrayNode, arraySeparator2 + 1);
                }
            }
            else
                return NULL;
        }
        else
        {
            /* invalid name */
            return NULL;
        }
    }

    int nameElementLength = 0;

    if (separator)
        nameElementLength = (separator - name);
    else
        nameElementLength = strlen(name);

    ModelNode* nextNode = self->firstChild;

    ModelNode* matchingNode = NULL;

    while (nextNode)
    {
        if (nextNode->name == NULL)
        {
            break; /* is an array element */
        }

        int nodeNameLen = strlen(nextNode->name);

        if (nodeNameLen == nameElementLength)
        {
            if (memcmp(nextNode->name, name, nodeNameLen) == 0)
            {
                matchingNode = nextNode;
                break;
            }
        }

        nextNode = nextNode->sibling;
    }

    if ((separator != NULL) && (matchingNode != NULL))
    {
        return ModelNode_getChild(matchingNode, separator + 1);
    }
    else
        return matchingNode;
}

ModelNode*
ModelNode_getChildWithIdx(ModelNode* self, int idx)
{
    ModelNode* foundElement = NULL;

    if (self->modelType == DataObjectModelType || self->modelType == DataAttributeModelType)
    {
        ModelNode* nextNode = self->firstChild;

        int currentIdx = 0;

        while (nextNode)
        {
            if (currentIdx == idx)
            {
                foundElement = nextNode;
                break;
            }

            currentIdx++;

            nextNode = nextNode->sibling;
        }
    }

    return foundElement;
}

ModelNode*
ModelNode_getChildWithFc(ModelNode* self, const char* name, FunctionalConstraint fc)
{
    /* check for separator */
    const char* separator = strchr(name, '.');

    int nameElementLength = 0;

    if (separator)
        nameElementLength = (separator - name);
    else
        nameElementLength = strlen(name);

    ModelNode* nextNode = self->firstChild;

    ModelNode* matchingNode = NULL;

    while (nextNode)
    {
        int nodeNameLen = strlen(nextNode->name);

        if (nodeNameLen == nameElementLength)
        {
            if (memcmp(nextNode->name, name, nodeNameLen) == 0)
            {
                if (separator == NULL)
                {
                    if (nextNode->modelType == DataAttributeModelType)
                    {
                        DataAttribute* da = (DataAttribute*)nextNode;

                        if (da->fc == fc)
                        {
                            matchingNode = nextNode;
                            break;
                        }
                    }
                }
                else
                {
                    if (nextNode->modelType == DataAttributeModelType)
                    {
                        DataAttribute* da = (DataAttribute*)nextNode;

                        if (da->fc == fc)
                        {
                            matchingNode = nextNode;
                            break;
                        }
                    }
                    else
                    {
                        matchingNode = nextNode;
                        break;
                    }
                }
            }
        }

        nextNode = nextNode->sibling;
    }

    if ((separator != NULL) && (matchingNode != NULL))
    {
        return ModelNode_getChildWithFc(matchingNode, separator + 1, fc);
    }
    else
        return matchingNode;
}

ModelNodeType
ModelNode_getType(ModelNode* self)
{
    return self->modelType;
}

const char*
ModelNode_getName(ModelNode* self)
{
    return self->name;
}

ModelNode*
ModelNode_getParent(ModelNode* self)
{
    return self->parent;
}

LinkedList
ModelNode_getChildren(ModelNode* self)
{
    LinkedList childNodes = NULL;

    if (self->firstChild)
        childNodes = LinkedList_create();

    ModelNode* childNode = self->firstChild;

    while (childNode)
    {
        LinkedList_add(childNodes, childNode);

        childNode = childNode->sibling;
    }

    return childNodes;
}

LogicalNode*
LogicalDevice_getLogicalNode(LogicalDevice* self, const char* nodeName)
{
    return (LogicalNode*)ModelNode_getChild((ModelNode*)self, nodeName);
}

SettingGroupControlBlock*
LogicalDevice_getSettingGroupControlBlock(LogicalDevice* self)
{
    IedModel* model = (IedModel*)self->parent;

    if (model == NULL)
        return NULL;

    LogicalNode* ln = LogicalDevice_getLogicalNode(self, "LLN0");

    if (ln == NULL)
    {
        if (DEBUG_IED_SERVER)
            printf("IED_SERVER: logical node LLN0 not found!\n");

        return NULL;
    }

    SettingGroupControlBlock* sgcb = model->sgcbs;

    while (sgcb)
    {
        if (sgcb->parent == ln)
            return sgcb;

        sgcb = sgcb->sibling;
    }

    return NULL;
}
