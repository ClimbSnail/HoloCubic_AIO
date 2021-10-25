#include "file_msg.h"
#include <string.h>

MsgHead::MsgHead(MODULE_TYPE from_who, MODULE_TYPE to_who)
{
    m_header_mark = 0x2323;
    m_from_who = from_who;
    m_to_who = to_who;
    m_msg_len = 6;
}

uint32_t MsgHead::decode(const uint8_t *msg)
{
    if (NULL == msg)
    {
        return 0;
    }
    m_from_who = (MODULE_TYPE)msg[2];
    m_to_who = (MODULE_TYPE)msg[3];
    m_msg_len = msg[4] << 8 + msg[5];
    // m_msg_len = ntohs(msg[4] << 8 + msg[5]);
    return 6;
}

uint32_t MsgHead::encode(uint8_t *msg)
{
    if (NULL == msg)
    {
        return 0;
    }
    msg[0] = 0x23;
    msg[1] = 0x23;
    msg[2] = (uint8_t)m_from_who;
    msg[3] = (uint8_t)m_to_who;
    msg[4] = (uint8_t)(m_msg_len >> 8);
    msg[5] = (uint8_t)(m_msg_len & 0x00FF);
    // uint16_t msg_len = htons(m_msg_len);
    // memcpy((uint8_t *)&msg_len, &msg[4], 2);
    return 6;
}

/********************************************************/
/* FileSystem
*********************************************************/

FileSystem::FileSystem(ACTION_TYPE action_type)
{
    m_msg_head.m_from_who = MODULE_TYPE_CUBIC_FILE_MANAGER;
    m_msg_head.m_to_who = MODULE_TYPE_C_FILE_MANAGER;
    m_action_type = action_type;
};

uint32_t FileSystem::decode(const uint8_t *msg)
{
    if (NULL == msg)
    {
        return 0;
    }
    uint32_t index = m_msg_head.decode(msg);
    m_action_type = (ACTION_TYPE)msg[index];
    return index + 1;
}

uint32_t FileSystem::encode(uint8_t *msg)
{
    if (NULL == msg)
    {
        return false;
    }
    uint32_t index = m_msg_head.encode(msg);
    msg[index] = (uint8_t)m_action_type;
    return index + 1;
}

/********************************************************/
/* DirCreate
*********************************************************/

DirCreate::DirCreate(const char *dir_name)
{
    m_file_system.m_action_type = AT_DIR_CREATE;
    strncpy(m_dir_path, dir_name, 99);
};

uint32_t DirCreate::decode(const uint8_t *msg)
{
    if (NULL == msg)
    {
        return 0;
    }
    uint32_t index = m_file_system.decode(msg);
    memcpy(m_dir_path, (char *)&msg[index], 99);
    return index + strlen((char *)&msg[index]);
}

uint32_t DirCreate::encode(uint8_t *msg)
{
    if (NULL == msg)
    {
        return false;
    }
    uint32_t index = m_file_system.encode(msg);
    strncpy((char *)&msg[index], m_dir_path, 99);
    return index + strlen(m_dir_path);
}

/********************************************************/
/* DirList
*********************************************************/

DirList::DirList(const char *dir_path, const char *dir_info)
{
    m_file_system.m_action_type = AT_DIR_LIST;

    if (NULL != dir_path)
    {
        memcpy(m_dir_path, dir_path, 99);
    }
    if (NULL != dir_info)
    {
        memcpy(m_dir_info, dir_info, 400);
    }
};

uint32_t DirList::decode(const uint8_t *msg)
{
    if (NULL == msg)
    {
        return 0;
    }
    uint32_t index = m_file_system.decode(msg);
    memcpy(m_dir_path, (char *)&msg[index], 99);
    memcpy(m_dir_info, (char *)&msg[index + 99], 400);
    m_dir_path[98] = 0;
    m_dir_info[399] = 0;
    return index + 499;
}

uint32_t DirList::encode(uint8_t *msg)
{
    if (NULL == msg)
    {
        return false;
    }
    uint32_t index = m_file_system.encode(msg);
    strncpy((char *)&msg[index], m_dir_path, 99);
    strncpy((char *)&msg[index + 99], m_dir_info, 400);
    m_file_system.m_msg_head.m_msg_len = index + 499;
    m_file_system.encode(msg);
    return index + 499;
}