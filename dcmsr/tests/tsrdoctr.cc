/*
 *
 *  Copyright (C) 2014, OFFIS e.V.
 *  All rights reserved.  See COPYRIGHT file for details.
 *
 *  This software and supporting documentation were developed by
 *
 *    OFFIS e.V.
 *    R&D Division Health
 *    Escherweg 2
 *    D-26121 Oldenburg, Germany
 *
 *
 *  Module: dcmsr
 *
 *  Author: Joerg Riesmeier
 *
 *  Purpose:
 *    test program for classes DSRDocumentTree, DSRDocumentTreeNode and DSRContentItem
 *
 */


#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */

#include "dcmtk/ofstd/oftest.h"
#include "dcmtk/dcmsr/dsrdoc.h"
#include "dcmtk/dcmsr/dsrnumtn.h"
#include "dcmtk/dcmsr/dsrtextn.h"


OFTEST(dcmsr_addContentItem)
{
    /* first, create an SR document to get an empty SR tree */
    DSRDocument doc(DSRTypes::DT_ComprehensiveSR);
    DSRDocumentTree &tree = doc.getTree();
    /* then try to add some content items */
    OFCHECK(tree.addContentItem(DSRTypes::RT_isRoot, DSRTypes::VT_Container));
    /* add content item with given pointer */
    DSRNumTreeNode *numNode = new DSRNumTreeNode(DSRTypes::RT_contains);
    OFCHECK_EQUAL(tree.addContentItem(numNode, DSRTypes::AM_belowCurrent), numNode);
    if (numNode != NULL)
    {
        OFCHECK(numNode->getRelationshipType() == DSRTypes::RT_contains);
        OFCHECK(numNode->getValueType() == DSRTypes::VT_Num);
        OFCHECK(numNode->setConceptName(DSRCodedEntryValue("121206", "DCM", "Distance")).good());
        OFCHECK(numNode->setValue("1.5", DSRCodedEntryValue("cm", "UCUM", "centimeter")).good());
        OFCHECK(numNode->setRationalRepresentation(3, 2).good());
        OFCHECK(numNode->setFloatingPointRepresentation(1.5).good());
        /* the following code usually implies "Measurement not available", but this is only a test */
        OFCHECK(numNode->setNumericValueQualifier(DSRCodedEntryValue("114006", "DCM", "Measurement failure")).good());
    } else
        OFCHECK_FAIL("could not create new NUM content item");
    /* try to add content items that should fail */
    OFCHECK(tree.addContentItem(NULL) == NULL);
    OFCHECK(tree.addContentItem(DSRTypes::createDocumentTreeNode(DSRTypes::RT_hasProperties, DSRTypes::VT_Text), DSRTypes::AM_afterCurrent, OFTrue /*deleteIfFail*/) == NULL);
    /* NB: this test program does not always delete allocated memory (if adding a node fails) */
}


OFTEST(dcmsr_copyContentItem)
{
    /* first, create a new SR document */
    DSRDocument doc(DSRTypes::DT_ComprehensiveSR);
    DSRDocumentTree &tree = doc.getTree();
    /* then add some content items */
    OFCHECK(tree.addContentItem(DSRTypes::RT_isRoot, DSRTypes::VT_Container));
    OFCHECK(tree.addContentItem(DSRTypes::RT_contains, DSRTypes::VT_Text, DSRTypes::AM_belowCurrent));
    OFCHECK(tree.getCurrentContentItem().setStringValue("Sample text").good());
    OFCHECK(tree.getCurrentContentItem().setConceptName(DSRCodedEntryValue("121111", "DCM", "Summary")).good());
    OFCHECK(tree.addContentItem(DSRTypes::RT_contains, DSRTypes::VT_Num, DSRTypes::AM_afterCurrent));
    /* create a copy of the current content item */
    DSRContentItem item(tree.getCurrentContentItem());
    OFCHECK_EQUAL(item.getValueType(), DSRTypes::VT_Num);
    OFCHECK_EQUAL(item.getRelationshipType(), DSRTypes::RT_contains);
    /* clone the previous content item */
    OFCHECK(tree.gotoPrevious() > 0);
    DSRDocumentTreeNode *treeNode = tree.cloneCurrentTreeNode();
    if (treeNode != NULL)
    {
        /* and check some details */
        if (treeNode->getValueType() == DSRTypes::VT_Text)
        {
            DSRTextTreeNode *textNode = OFstatic_cast(DSRTextTreeNode *, treeNode);
            OFCHECK_EQUAL(textNode->getValue(), "Sample text");
        } else
            OFCHECK_FAIL("clone of TEXT content item has wrong value type");
        OFCHECK_EQUAL(treeNode->getRelationshipType(), DSRTypes::RT_contains);
        OFCHECK_EQUAL(treeNode->getConceptName().getCodeMeaning(), "Summary");
        delete treeNode;
    } else
        OFCHECK_FAIL("could not create clone of TEXT content item");
}


OFTEST(dcmsr_gotoNamedNode)
{
    /* first, create a new SR document */
    DSRDocument doc(DSRTypes::DT_ComprehensiveSR);
    DSRDocumentTree &tree = doc.getTree();
    /* then add some content items */
    OFCHECK(tree.addContentItem(DSRTypes::RT_isRoot, DSRTypes::VT_Container, DSRCodedEntryValue("121111", "DCM", "Summary")).good());
    OFCHECK(tree.addChildContentItem(DSRTypes::RT_contains, DSRTypes::VT_Num, DSRCodedEntryValue("121206", "DCM", "Distance")).good());
    const size_t nodeID = tree.getNodeID();
    OFCHECK(tree.addContentItem(DSRTypes::RT_contains, DSRTypes::VT_Text, DSRCodedEntryValue("1234", "99_PRV", "NOS")).good());
    /* and check the "search by name" function */
    OFCHECK_EQUAL(tree.gotoNamedNode(DSRCodedEntryValue("121206", "DCM", "Distance")), nodeID);
    OFCHECK_EQUAL(tree.gotoNamedNode(DSRCodedEntryValue("1234", "99_PRV", "NOS")), nodeID + 1);
}


OFTEST(dcmsr_createSubTree)
{
    /* first, create an empty document subtree */
    DSRDocumentSubTree tree;
    /* then try to add some content items (no dedicated root) */
    OFCHECK(tree.addContentItem(DSRTypes::RT_unknown, DSRTypes::VT_Text));
    OFCHECK(tree.addContentItem(DSRTypes::RT_unknown, DSRTypes::VT_Num));
    OFCHECK(tree.addContentItem(DSRTypes::RT_hasProperties, DSRTypes::VT_Code, DSRTypes::AM_belowCurrent));
    /* try to add content items that should fail */
    OFCHECK(tree.addContentItem(DSRTypes::RT_invalid, DSRTypes::VT_Date) == 0);
    OFCHECK(tree.addContentItem(DSRTypes::RT_contains, DSRTypes::VT_invalid) == 0);
    OFCHECK(tree.addContentItem(DSRTypes::RT_invalid, DSRTypes::VT_invalid) == 0);
}