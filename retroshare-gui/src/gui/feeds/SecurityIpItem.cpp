/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2015, RetroShare Team
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 *  Boston, MA  02110-1301, USA.
 ****************************************************************/

#include <QDateTime>
#include <QTimer>

#include "SecurityIpItem.h"
#include "FeedHolder.h"
#include"ui_SecurityIpItem.h"
#include "retroshare-gui/RsAutoUpdatePage.h"
#include "gui/connect/ConfCertDialog.h"
#include "util/DateTime.h"
#include "gui/common/PeerDefs.h"
#include "gui/common/RsBanListDefs.h"

#include <retroshare/rspeers.h>
#include <retroshare/rsbanlist.h>

/*****
 * #define DEBUG_ITEM 1
 ****/

/** Constructor */
SecurityIpItem::SecurityIpItem(FeedHolder *parent, const RsPeerId &sslId, const std::string &ipAddr, uint32_t result, bool isTest) :
    FeedItem(NULL), mParent(parent), mSslId(sslId), mIPAddr(ipAddr), mResult(result), mIsTest(isTest),
    ui(new(Ui::SecurityIpItem))
{
	/* Invoke the Qt Designer generated object setup routine */
	ui->setupUi(this);

	ui->peerDetailsButton->setEnabled(false);

	/* general ones */
	connect(ui->expandButton, SIGNAL(clicked(void)), this, SLOT(toggle(void)));
	connect(ui->clearButton, SIGNAL(clicked(void)), this, SLOT(removeItem(void)));

	/* specific ones */
	connect(ui->peerDetailsButton, SIGNAL(clicked()), this, SLOT(peerDetails()));

	ui->avatar->setId(ChatId(mSslId));
	ui->rsBanListButton->setMode(RsBanListToolButton::LIST_WHITELIST, RsBanListToolButton::MODE_ADD);

	ui->expandFrame->hide();

	updateItemStatic();
	updateItem();
}

bool SecurityIpItem::isSame(const RsPeerId &sslId)
{
	if ((mSslId == sslId)) {
		return true;
	}

	return false;
}

void SecurityIpItem::updateItemStatic()
{
	if (!rsPeers)
		return;

	/* fill in */
#ifdef DEBUG_ITEM
	std::cerr << "SecurityIpItem::updateItemStatic()";
	std::cerr << std::endl;
#endif

	QDateTime currentTime = QDateTime::currentDateTime();
	ui->timeLabel->setText(DateTime::formatLongDateTime(currentTime.toTime_t()));
}

void SecurityIpItem::updateItem()
{
	if (!rsPeers)
		return;

	/* fill in */
#ifdef DEBUG_ITEM
	std::cerr << "SecurityIpItem::updateItem()";
	std::cerr << std::endl;
#endif

	if(!RsAutoUpdatePage::eventsLocked()) {
		ui->titleLabel->setText(RsBanListDefs::resultString(mResult));
		ui->ipAddr->setText(QString::fromStdString(mIPAddr));

		if (mIsTest) {
			ui->rsBanListButton->setEnabled(false);
		} else {
			switch (mResult) {
			case RSBANLIST_CHECK_RESULT_NOCHECK:
			case RSBANLIST_CHECK_RESULT_NOT_WHITELISTED:
			case RSBANLIST_CHECK_RESULT_ACCEPTED:
				ui->rsBanListButton->hide();
				break;
			case RSBANLIST_CHECK_RESULT_BLACKLISTED:
				ui->rsBanListButton->setVisible(ui->rsBanListButton->setIpAddress(QString::fromStdString(mIPAddr)));
				break;
			default:
				ui->rsBanListButton->hide();
			}
		}

		RsPeerDetails details;
		if (!rsPeers->getPeerDetails(mSslId, details))
		{
			/* set peer name */
			ui->peer->setText(tr("Unknown Peer"));

			/* expanded Info */
			ui->peerID->setText(QString::fromStdString(mSslId.toStdString()));
			ui->peerName->setText(tr("Unknown Peer"));
			ui->locationLabel->setText(tr("Unknown Peer"));
		} else {
			/* set peer name */
			ui->peer->setText(PeerDefs::nameWithLocation(details));

			/* expanded Info */
			ui->peerID->setText(QString::fromStdString(details.id.toStdString()));
			ui->peerName->setText(QString::fromUtf8(details.name.c_str()));
			ui->location->setText(QString::fromUtf8(details.location.c_str()));

			/* Buttons */
			ui->peerDetailsButton->setEnabled(true);
		}
	}

	/* slow Tick  */
	int msec_rate = 10129;

	QTimer::singleShot( msec_rate, this, SLOT(updateItem(void)));
}

void SecurityIpItem::toggle()
{
	expand(ui->expandFrame->isHidden());
}

void SecurityIpItem::expand(bool open)
{
	if (mParent) {
		mParent->lockLayout(this, true);
	}

	if (open)
	{
		ui->expandFrame->show();
		ui->expandButton->setIcon(QIcon(":/images/edit_remove24.png"));
		ui->expandButton->setToolTip(tr("Hide"));
	}
	else
	{
		ui->expandFrame->hide();
		ui->expandButton->setIcon(QIcon(":/images/edit_add24.png"));
		ui->expandButton->setToolTip(tr("Expand"));
	}

	emit sizeChanged(this);

	if (mParent) {
		mParent->lockLayout(this, false);
	}
}

void SecurityIpItem::removeItem()
{
#ifdef DEBUG_ITEM
	std::cerr << "SecurityIpItem::removeItem()";
	std::cerr << std::endl;
#endif

	mParent->lockLayout(this, true);
	hide();
	mParent->lockLayout(this, false);

	if (mParent)
	{
		mParent->deleteFeedItem(this, mFeedId);
	}
}

///*********** SPECIFIC FUNCTIONS ***********************/

void SecurityIpItem::peerDetails()
{
#ifdef DEBUG_ITEM
	std::cerr << "SecurityIpItem::peerDetails()";
	std::cerr << std::endl;
#endif

	RsPeerDetails details;
	if (rsPeers->getPeerDetails(mSslId, details)) {
		ConfCertDialog::showIt(mSslId, ConfCertDialog::PageDetails);
	}
}